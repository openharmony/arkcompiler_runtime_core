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

#include "runtime/mem/gc/gc_root.h"

#include "libpandafile/panda_cache.h"
#include "runtime/include/object_header.h"
#include "runtime/include/runtime.h"
#include "runtime/include/stack_walker-inl.h"
#include "runtime/mem/refstorage/reference_storage.h"
#include "runtime/mem/gc/card_table-inl.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/gc_root_type.h"
#include "runtime/mem/object_helpers.h"
#include "runtime/mem/refstorage/global_object_storage.h"
#include "runtime/include/panda_vm.h"

namespace panda::mem {

GCRoot::GCRoot(RootType type, ObjectHeader *obj)
{
    type_ = type;
    from_object_ = nullptr;
    object_ = obj;
}

GCRoot::GCRoot(RootType type, ObjectHeader *from_object, ObjectHeader *obj)
{
    ASSERT((from_object != nullptr && type == RootType::ROOT_TENURED) || type != RootType::ROOT_TENURED);
    type_ = type;
    from_object_ = from_object;
    object_ = obj;
}

RootType GCRoot::GetType() const
{
    return type_;
}

ObjectHeader *GCRoot::GetObjectHeader() const
{
    return object_;
}

ObjectHeader *GCRoot::GetFromObjectHeader() const
{
    ASSERT((from_object_ != nullptr && type_ == RootType::ROOT_TENURED) || type_ != RootType::ROOT_TENURED);
    return from_object_;
}

std::ostream &operator<<(std::ostream &os, const GCRoot &root)
{
    switch (root.GetType()) {
        case RootType::ROOT_CLASS:
            os << "ROOT CLASS";
            break;
        case RootType::ROOT_FRAME:
            os << "ROOT FRAME";
            break;
        case RootType::ROOT_THREAD:
            os << "ROOT THREAD";
            break;
        case RootType::ROOT_TENURED:
            os << "ROOT TENURED";
            break;
        case RootType::ROOT_NATIVE_GLOBAL:
            os << "ROOT NATIVE_GLOBAL";
            break;
        case RootType::ROOT_NATIVE_LOCAL:
            os << "ROOT NATIVE_LOCAL";
            break;
        case RootType::ROOT_AOT_STRING_SLOT:
            os << "ROOT AOT_STRING_SLOT";
            break;
        case RootType::ROOT_UNKNOWN:
        default:
            LOG(FATAL, GC) << "ROOT UNKNOWN";
            break;
    }
    os << std::hex << " " << root.GetObjectHeader() << std::endl;
    return os;
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::VisitNonHeapRoots(const GCRootVisitor &gc_root_visitor, VisitGCRootFlags flags) const
{
    VisitLocalRoots(gc_root_visitor);
    VisitClassRoots(gc_root_visitor, flags);
    VisitAotStringRoots(gc_root_visitor, flags);
    VisitClassLinkerContextRoots(gc_root_visitor);
    VisitVmRoots(gc_root_visitor);
    auto *storage = vm_->GetGlobalObjectStorage();
    if (storage != nullptr) {
        storage->VisitObjects(gc_root_visitor, mem::RootType::ROOT_NATIVE_GLOBAL);
    }
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::VisitCardTableRoots(CardTable *card_table, ObjectAllocatorBase *allocator,
                                                      GCRootVisitor root_visitor, MemRangeChecker range_checker,
                                                      ObjectChecker range_object_checker,
                                                      ObjectChecker from_object_checker, uint32_t processed_flag) const
{
    card_table->VisitMarked(
        [&allocator, &root_visitor, &range_checker, &range_object_checker, &from_object_checker,
         &card_table](MemRange mem_range) {
            if (range_checker(mem_range)) {
                auto objects_in_range_visitor = [&root_visitor, &range_object_checker,
                                                 &from_object_checker](ObjectHeader *object_header) {
                    auto traverse_object_in_range = [&root_visitor, &range_object_checker](
                                                        ObjectHeader *from_object, ObjectHeader *object_to_traverse) {
                        if (range_object_checker(object_to_traverse)) {
                            // The weak references from dynobjects should not be regarded as roots.
                            TaggedValue value(object_to_traverse);
                            if (!value.IsWeak()) {
                                root_visitor(GCRoot(RootType::ROOT_TENURED, from_object, object_to_traverse));
                            }
                        }
                    };
                    if (object_header->ClassAddr<BaseClass>() != nullptr && from_object_checker(object_header)) {
                        // The class may be null in the situation when a new objct is allocated in the card
                        // we are visiting now, but the class is not set yet.
                        ObjectHelpers<LanguageConfig::LANG_TYPE>::TraverseAllObjects(object_header,
                                                                                     traverse_object_in_range);
                    }
                };
                allocator->IterateOverObjectsInRange(mem_range, objects_in_range_visitor);
            } else {
                card_table->MarkCard(mem_range.GetStartAddress());
            }
        },
        processed_flag);
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::VisitRootsForThread(ManagedThread *thread, const GCRootVisitor &gc_root_visitor) const
{
    LOG(DEBUG, GC) << "Start collecting roots for thread " << thread->GetId();

    thread->VisitGCRoots([&gc_root_visitor](ObjectHeader *obj) {
        LOG(DEBUG, GC) << " Found root for thread" << GetDebugInfoAboutObject(obj);
        gc_root_visitor({RootType::ROOT_THREAD, obj});
    });
    LOG(DEBUG, GC) << "Finish collecting roots for thread " << thread->GetId();
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::VisitLocalRoots(const GCRootVisitor &gc_root_visitor) const
{
    auto thread_visitor = [this, &gc_root_visitor](ManagedThread *thread) {
        VisitRootsForThread(thread, gc_root_visitor);
        for (auto stack = StackWalker::Create(thread); stack.HasFrame(); stack.NextFrame()) {
            LOG(DEBUG, GC) << " VisitRoots frame " << std::hex << stack.GetFp();
            stack.IterateObjects([this, &gc_root_visitor](auto &vreg) {
                this->VisitRegisterRoot(vreg, gc_root_visitor);
                return true;
            });
        }
        return true;
    };
    if constexpr (LanguageConfig::MT_MODE == MT_MODE_MULTI) {  // NOLINT
        vm_->GetThreadManager()->EnumerateThreads(thread_visitor);
    } else {  // NOLINT
        thread_visitor(vm_->GetAssociatedThread());
    }
}

template <class LanguageConfig>
template <class VRegRef>
void RootManager<LanguageConfig>::VisitRegisterRoot(const VRegRef &v_register,
                                                    const GCRootVisitor &gc_root_visitor) const
{
    if (UNLIKELY(v_register.HasObject())) {
        ObjectHeader *object_header = v_register.GetReference();
        if (object_header != nullptr) {
            LOG(DEBUG, GC) << " Found root for register" << GetDebugInfoAboutObject(object_header);
            gc_root_visitor({RootType::ROOT_FRAME, object_header});
        }
    }
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::VisitVmRoots(const GCRootVisitor &gc_root_visitor) const
{
    vm_->VisitVmRoots(gc_root_visitor);
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::VisitAotStringRoots(const GCRootVisitor &gc_root_visitor,
                                                      VisitGCRootFlags flags) const
{
    trace::ScopedTrace scoped_trace(__FUNCTION__);
    LOG(DEBUG, GC) << "Start collecting AOT string slot roots";
    Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->VisitAotStringRoots(
        [&gc_root_visitor](ObjectHeader **slot) {
            gc_root_visitor({RootType::ROOT_AOT_STRING_SLOT, *slot});
        },
        (flags & VisitGCRootFlags::ACCESS_ROOT_AOT_STRINGS_ONLY_YOUNG) != 0);
    LOG(DEBUG, GC) << "Finish collecting AOT string slot roots";
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::UpdateAotStringRoots()
{
    trace::ScopedTrace scoped_trace(__FUNCTION__);
    LOG(DEBUG, GC) << "=== AOT string slot roots update. BEGIN ===";
    auto oa = Thread::GetCurrent()->GetVM()->GetHeapManager()->GetObjectAllocator().AsObjectAllocator();
    Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->UpdateAotStringRoots(
        [](ObjectHeader **root) {
            auto root_value = *root;
            if (root_value->IsForwarded()) {
                *root = ::panda::mem::GetForwardAddress(root_value);
            }
        },
        [&oa](const ObjectHeader *root) {
            return oa->HasYoungSpace() && oa->IsAddressInYoungSpace(reinterpret_cast<uintptr_t>(root));
        });
    LOG(DEBUG, GC) << "=== AOT string slot roots update. END ===";
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::UpdateVmRefs()
{
    vm_->UpdateVmRefs();
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::UpdateGlobalObjectStorage()
{
    auto global_storage = vm_->GetGlobalObjectStorage();
    if (global_storage != nullptr) {
        global_storage->UpdateMovedRefs();
    }
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::VisitClassRoots(const GCRootVisitor &gc_root_visitor, VisitGCRootFlags flags) const
{
    LOG(DEBUG, GC) << "Start collecting roots for classes";
    auto class_linker = Runtime::GetCurrent()->GetClassLinker();
    auto class_root_visitor = [&gc_root_visitor](Class *cls) {
        gc_root_visitor({RootType::ROOT_CLASS, cls->GetManagedObject()});
        LOG(DEBUG, GC) << " Found class root " << GetDebugInfoAboutObject(cls->GetManagedObject());
        return true;
    };
    auto *extension = class_linker->GetExtension(LanguageConfig::LANG);
    extension->EnumerateClasses(class_root_visitor, flags);

    // TODO(maksenov): Remove after supporting multiple GC instances
    if (LanguageConfig::LANG != panda_file::SourceLang::PANDA_ASSEMBLY &&
        class_linker->HasExtension(panda_file::SourceLang::PANDA_ASSEMBLY)) {
        class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->EnumerateClasses(class_root_visitor, flags);
    }

    LOG(DEBUG, GC) << "Finish collecting roots for classes";
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::UpdateThreadLocals()
{
    LOG(DEBUG, GC) << "=== ThreadLocals Update moved. BEGIN ===";
    if constexpr (LanguageConfig::MT_MODE == MT_MODE_MULTI) {  // NOLINT
        vm_->GetThreadManager()->EnumerateThreads([](MTManagedThread *thread) {
            thread->UpdateGCRoots();
            return true;
        });
    } else {  // NOLINT
        vm_->GetAssociatedThread()->UpdateGCRoots();
    }
    LOG(DEBUG, GC) << "=== ThreadLocals Update moved. END ===";
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::VisitClassLinkerContextRoots(const GCRootVisitor &gc_root_visitor) const
{
    LOG(DEBUG, GC) << "Start collecting roots for class linker contexts";
    auto class_linker = Runtime::GetCurrent()->GetClassLinker();
    auto *extension = class_linker->GetExtension(LanguageConfig::LANG);
    extension->EnumerateContexts([&gc_root_visitor](ClassLinkerContext *ctx) {
        ctx->VisitGCRoots([&gc_root_visitor](ObjectHeader *obj) {
            LOG(DEBUG, GC) << " Found root for class linker context " << GetDebugInfoAboutObject(obj);
            gc_root_visitor({RootType::ROOT_CLASS_LINKER, obj});
        });
        return true;
    });
    LOG(DEBUG, GC) << "Finish collecting roots for class linker contexts";
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::UpdateClassLinkerContextRoots()
{
    auto class_linker = Runtime::GetCurrent()->GetClassLinker();
    auto *extension = class_linker->GetExtension(LanguageConfig::LANG);
    extension->EnumerateContexts([](ClassLinkerContext *ctx) {
        ctx->UpdateGCRoots();
        return true;
    });
}

uint32_t operator&(VisitGCRootFlags left, VisitGCRootFlags right)
{
    return static_cast<uint32_t>(left) & static_cast<uint32_t>(right);
}

VisitGCRootFlags operator|(VisitGCRootFlags left, VisitGCRootFlags right)
{
    return static_cast<VisitGCRootFlags>(static_cast<uint32_t>(left) | static_cast<uint32_t>(right));
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(RootManager);

}  // namespace panda::mem
