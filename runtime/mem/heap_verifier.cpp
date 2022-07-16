/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/gc/gc_root.h"
#include "runtime/mem/heap_verifier.h"

namespace panda::mem {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_HEAP_VERIFIER LOG(ERROR, GC) << "HEAP_VERIFIER: "

// Should be called only with MutatorLock held
template <class LanguageConfig>
size_t HeapVerifier<LanguageConfig>::VerifyAllPaused() const
{
    Rendezvous *rendezvous = Thread::GetCurrent()->GetVM()->GetRendezvous();
    rendezvous->SafepointBegin();
    size_t fail_count = VerifyAll();
    rendezvous->SafepointEnd();
    return fail_count;
}

template <LangTypeT LangType>
void HeapObjectVerifier<LangType>::operator()(ObjectHeader *obj)
{
    HeapReferenceVerifier<LangType> ref_verifier(HEAP, FAIL_COUNT);
    ObjectHelpers<LangType>::TraverseAllObjects(obj, ref_verifier);
}

template <LangTypeT LangType>
void HeapReferenceVerifier<LangType>::operator()([[maybe_unused]] ObjectHeader *object_header, ObjectHeader *referent)
{
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (LangType == LANG_TYPE_DYNAMIC) {
        // Weak reference can be passed here, need to resolve the referent
        coretypes::TaggedValue value(referent);
        if (value.IsWeak()) {
            referent = value.GetWeakReferent();
        }
    }
    auto obj_allocator = HEAP->GetObjectAllocator().AsObjectAllocator();
    if (!obj_allocator->IsLive(referent)) {
        LOG_HEAP_VERIFIER << "Heap corruption found! Heap object " << std::hex << object_header
                          << " references a dead object at " << referent;
        ++(*FAIL_COUNT);
    } else if (referent->IsForwarded()) {
        LOG_HEAP_VERIFIER << "Heap corruption found! Heap object " << std::hex << object_header
                          << " references a forwarded object at " << referent;
        ++(*FAIL_COUNT);
    }
}

template <LangTypeT LangType>
void HeapReferenceVerifier<LangType>::operator()(const GCRoot &root)
{
    auto obj_allocator = HEAP->GetObjectAllocator().AsObjectAllocator();
    auto referent = root.GetObjectHeader();
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (LangType == LANG_TYPE_DYNAMIC) {
        // Weak reference can be passed here, need to resolve the referent
        coretypes::TaggedValue value(referent);
        if (value.IsWeak()) {
            referent = value.GetWeakReferent();
        }
    }
    if (!obj_allocator->IsLive(referent)) {
        LOG_HEAP_VERIFIER << "Heap corruption found! Root references a dead object at " << std::hex << referent;
        ++(*FAIL_COUNT);
    } else if (referent->IsForwarded()) {
        LOG_HEAP_VERIFIER << "Heap corruption found! Root references a forwarded object at " << std::hex << referent;
        ++(*FAIL_COUNT);
    }
}

template <class LanguageConfig>
bool HeapVerifier<LanguageConfig>::IsValidObjectAddress(void *addr) const
{
    return IsAligned<DEFAULT_ALIGNMENT_IN_BYTES>(ToUintPtr(addr)) && IsHeapAddress(addr);
}

template <class LanguageConfig>
bool HeapVerifier<LanguageConfig>::IsHeapAddress(void *addr) const
{
    return heap_->GetObjectAllocator().AsObjectAllocator()->ContainObject(reinterpret_cast<ObjectHeader *>(addr));
}

template <class LanguageConfig>
size_t HeapVerifier<LanguageConfig>::VerifyHeap() const
{
    return heap_->VerifyHeapReferences();
}

template <class LanguageConfig>
size_t HeapVerifier<LanguageConfig>::VerifyRoot() const
{
    RootManager<LanguageConfig> root_manager;
    size_t fail_count = 0;
    root_manager.SetPandaVM(heap_->GetPandaVM());
    root_manager.VisitNonHeapRoots([this, &fail_count](const GCRoot &root) {
        if (root.GetType() == RootType::ROOT_FRAME || root.GetType() == RootType::ROOT_THREAD) {
            auto *base_cls = root.GetObjectHeader()->ClassAddr<BaseClass>();
            if (base_cls == nullptr) {
                LOG_HEAP_VERIFIER << "Heap corruption found! Class address for root " << std::hex
                                  << root.GetObjectHeader() << " is null";
                ++fail_count;
            } else if (!(!base_cls->IsDynamicClass() && static_cast<Class *>(base_cls)->IsClassClass())) {
                HeapReferenceVerifier<LanguageConfig::LANG_TYPE>(heap_, &fail_count)(root);
            }
        }
    });

    return fail_count;
}

template <class LanguageConfig>
size_t FastHeapVerifier<LanguageConfig>::VerifyAll() const
{
    PandaUnorderedSet<const ObjectHeader *> heap_objects;
    PandaVector<ObjectCache> referent_objects;
    size_t fails_count = 0;

    auto lazy_verify = [&](const ObjectHeader *object_header, const ObjectHeader *referent) {
        // Lazy verify during heap objects collection
        if (heap_objects.find(referent) == heap_objects.end()) {
            referent_objects.push_back(ObjectCache({object_header, referent}));
        }
        if (object_header->IsForwarded()) {
            LOG_HEAP_VERIFIER << "Heap object " << std::hex << object_header << " is forwarded object";
            ++fails_count;
        }
        auto *class_addr = object_header->ClassAddr<BaseClass>();
        if (!IsInObjectsAddressSpace(class_addr)) {
            LOG_HEAP_VERIFIER << "Heap object " << std::hex << object_header
                              << " has non-heap class address: " << class_addr;
            ++fails_count;
        }
    };
    const std::function<void(ObjectHeader *, ObjectHeader *)> lazy_verify_functor(lazy_verify);
    auto collect_objects = [&](ObjectHeader *object) {
        heap_objects.insert(object);
        ObjectHelpers<LanguageConfig::LANG_TYPE>::TraverseAllObjects(object, lazy_verify_functor);
    };

    // Heap objects verifier

    // Add strings from string table because these objects are like a phenix.
    // A string object may exist but there are no live references to it (no bit set in the live bitmap).
    // But later code may reuse it by calling StringTable::GetOrInternString so this string
    // get alive. That is why we mark all strings as alive by visiting the string table.
    Thread::GetCurrent()->GetVM()->GetStringTable()->VisitStrings(collect_objects);
    heap_->GetObjectAllocator().AsObjectAllocator()->IterateOverObjects(collect_objects);
    for (auto object_cache : referent_objects) {
        if (heap_objects.find(object_cache.referent) == heap_objects.end()) {
            LOG_HEAP_VERIFIER << "Heap object " << std::hex << object_cache.heap_object
                              << " references a dead object at " << object_cache.referent;
            ++fails_count;
        }
    }
    // Stack verifier
    RootManager<LanguageConfig> root_manager;
    root_manager.SetPandaVM(heap_->GetPandaVM());
    auto root_verifier = [&](const GCRoot &root) {
        const auto *root_obj_header = root.GetObjectHeader();
        auto *base_cls = root_obj_header->ClassAddr<BaseClass>();
        if (!IsAddressInObjectsHeap(ToUintPtr(base_cls))) {
            LOG_HEAP_VERIFIER << "Class address for root " << std::hex << root_obj_header
                              << " is not in objects heap: " << base_cls;
            ++fails_count;
        } else if (base_cls->IsDynamicClass() || !static_cast<Class *>(base_cls)->IsClassClass()) {
            if (heap_objects.find(root_obj_header) == heap_objects.end()) {
                LOG_HEAP_VERIFIER << "Root references a dead object at " << std::hex << root_obj_header;
                ++fails_count;
            }
        }
    };
    root_manager.VisitLocalRoots(root_verifier);

    return fails_count;
}

ObjectVerificationInfo::ObjectVerificationInfo(ObjectHeader *referent)
    : class_address_(referent->ClassAddr<void *>()), old_address_(referent)
{
}

bool ObjectVerificationInfo::VerifyUpdatedRef(ObjectHeader *object_header, ObjectHeader *updated_ref,
                                              bool in_alive_space) const
{
    ObjectHeader *correct_address = old_address_;
    if (!in_alive_space) {
        if (!old_address_->IsForwarded()) {
            LOG_HEAP_VERIFIER << "Object " << std::hex << object_header << " had reference " << old_address_
                              << ", which is not forwarded, new reference address: " << updated_ref;
            return false;
        }
        correct_address = GetForwardAddress(old_address_);
    }
    if (correct_address != updated_ref) {
        LOG_HEAP_VERIFIER << "Object " << std::hex << object_header << " has incorrect updated reference "
                          << updated_ref << ", correct address: " << correct_address;
        return false;
    }
    void *new_class_addr = updated_ref->ClassAddr<void *>();
    if (new_class_addr != class_address_) {
        LOG_HEAP_VERIFIER << "Object " << std::hex << object_header << " has incorrect class address ("
                          << new_class_addr << ") in updated reference " << updated_ref
                          << ", class address before collection: " << class_address_;
        return false;
    }

    return true;
}

template <class LanguageConfig>
bool HeapVerifierIntoGC<LanguageConfig>::InCollectableSpace(const ObjectHeader *object) const
{
    for (const auto &mem_range : this->collectable_mem_ranges_) {
        if (mem_range.Contains(ToUintPtr(object))) {
            return true;
        }
    }
    return false;
}

template <class LanguageConfig>
bool HeapVerifierIntoGC<LanguageConfig>::InAliveSpace(const ObjectHeader *object) const
{
    for (const auto &mem_range : this->alive_mem_ranges_) {
        if (mem_range.Contains(ToUintPtr(object))) {
            return true;
        }
    }
    return false;
}

template <class LanguageConfig>
void HeapVerifierIntoGC<LanguageConfig>::AddToVerificationInfo(RefsVerificationInfo &verification_info,
                                                               size_t ref_number, ObjectHeader *object_header,
                                                               ObjectHeader *referent)
{
    if (this->InCollectableSpace(referent)) {
        ObjectVerificationInfo obj_info(referent);
        auto it = verification_info.find(object_header);
        if (it != verification_info.end()) {
            it->second.insert({ref_number, obj_info});
        } else {
            verification_info.insert({object_header, VerifyingRefs({{ref_number, obj_info}})});
        }
    }
}

template <class LanguageConfig>
void HeapVerifierIntoGC<LanguageConfig>::CollectVerificationInfo(PandaVector<MemRange> &&collectable_mem_ranges)
{
    auto *obj_allocator = heap_->GetObjectAllocator().AsObjectAllocator();
    size_t ref_number = 0;
    collectable_mem_ranges_ = std::move(collectable_mem_ranges);
    const std::function<void(ObjectHeader *, ObjectHeader *)> refs_collector =
        [this, &ref_number](ObjectHeader *object_header, ObjectHeader *referent) {
            if (this->InCollectableSpace(object_header)) {
                this->AddToVerificationInfo(this->collectable_verification_info_, ref_number, object_header, referent);
            } else {
                this->AddToVerificationInfo(this->permanent_verification_info_, ref_number, object_header, referent);
            }
            ++ref_number;
        };

    auto collect_functor = [&ref_number, &refs_collector](ObjectHeader *object) {
        ref_number = 0;
        ObjectHelpers<LanguageConfig::LANG_TYPE>::TraverseAllObjects(object, refs_collector);
    };
    obj_allocator->IterateOverObjects(collect_functor);
}

template <class LanguageConfig>
size_t HeapVerifierIntoGC<LanguageConfig>::VerifyAll(PandaVector<MemRange> &&alive_mem_ranges)
{
    size_t fails_count = 0U;
    size_t ref_number = 0U;
    auto *obj_allocator = heap_->GetObjectAllocator().AsObjectAllocator();
    alive_mem_ranges_ = std::move(alive_mem_ranges);
    auto it = permanent_verification_info_.begin();
    for (auto &info : collectable_verification_info_) {
        ObjectHeader *obj = info.first;
        if (obj->IsForwarded()) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            permanent_verification_info_[GetForwardAddress(obj)] = std::move(info.second);
        } else if (this->InAliveSpace(obj)) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            permanent_verification_info_[obj] = std::move(info.second);
        }
    }
    collectable_verification_info_.clear();
    const std::function<void(ObjectHeader *, ObjectHeader *)> non_young_checker =
        [this, &fails_count](const ObjectHeader *object_header, const ObjectHeader *referent) {
            if (this->InCollectableSpace(referent)) {
                LOG_HEAP_VERIFIER << "Object " << std::hex << object_header << " references a dead object " << referent
                                  << " after collection";
                ++fails_count;
            }
        };
    const std::function<void(ObjectHeader *, ObjectHeader *)> same_obj_checker =
        [this, &non_young_checker, &ref_number, &fails_count, &it](ObjectHeader *object_header,
                                                                   ObjectHeader *referent) {
            auto ref_it = it->second.find(ref_number);
            if (ref_it != it->second.end()) {
                if (!ref_it->second.VerifyUpdatedRef(object_header, referent, this->InAliveSpace(referent))) {
                    ++fails_count;
                }
            } else {
                non_young_checker(object_header, referent);
            }
            ++ref_number;
        };
    // Check references in alive objects
    ObjectVisitor traverse_alive_obj = [&non_young_checker, &same_obj_checker, &ref_number, this,
                                        &it](ObjectHeader *object) {
        if (this->InCollectableSpace(object) && !this->InAliveSpace(object)) {
            return;
        }
        it = this->permanent_verification_info_.find(object);
        if (it == this->permanent_verification_info_.end()) {
            ObjectHelpers<LanguageConfig::LANG_TYPE>::TraverseAllObjects(object, non_young_checker);
        } else {
            ref_number = 0U;
            ObjectHelpers<LanguageConfig::LANG_TYPE>::TraverseAllObjects(object, same_obj_checker);
        }
    };
    obj_allocator->IterateOverObjects(traverse_alive_obj);
    return fails_count;
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(HeapVerifier);
TEMPLATE_CLASS_LANGUAGE_CONFIG(FastHeapVerifier);
TEMPLATE_CLASS_LANGUAGE_CONFIG(HeapVerifierIntoGC);
}  // namespace panda::mem
