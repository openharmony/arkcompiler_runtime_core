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

#include "runtime/include/object_header.h"

#include "runtime/include/coretypes/array.h"
#include "runtime/include/coretypes/class.h"
#include "runtime/include/hclass.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/free_object.h"
#include "runtime/mem/vm_handle.h"
#include "runtime/monitor_pool.h"
#include "runtime/handle_base-inl.h"

namespace panda {

/* static */
ObjectHeader *ObjectHeader::CreateObject(panda::BaseClass *klass, bool non_movable)
{
    ASSERT(klass != nullptr);
#ifndef NDEBUG
    if (!klass->IsDynamicClass()) {
        auto cls = static_cast<panda::Class *>(klass);
        ASSERT(cls->IsInstantiable());
        ASSERT(!cls->IsArrayClass());
        ASSERT(!cls->IsStringClass());
    }
#endif

    size_t size = klass->GetObjectSize();
    ASSERT(size != 0);
    mem::HeapManager *heap_manager = Thread::GetCurrent()->GetVM()->GetHeapManager();
    ObjectHeader *obj {nullptr};
    if (LIKELY(!non_movable)) {
        obj = heap_manager->AllocateObject(klass, size);
    } else {
        obj = heap_manager->AllocateNonMovableObject(klass, size);
    }
    return obj;
}

/* static */
ObjectHeader *ObjectHeader::Create(BaseClass *klass)
{
    return CreateObject(klass, false);
}

/* static */
ObjectHeader *ObjectHeader::CreateNonMovable(BaseClass *klass)
{
    return CreateObject(klass, true);
}

uint32_t ObjectHeader::GetHashCodeFromMonitor(Monitor *monitor_p)
{
    return monitor_p->GetHashCode();
}

uint32_t ObjectHeader::GetHashCodeMTSingle()
{
    auto mark = GetMark();

    switch (mark.GetState()) {
        case MarkWord::STATE_UNLOCKED: {
            mark = mark.DecodeFromHash(GenerateHashCode());
            ASSERT(mark.GetState() == MarkWord::STATE_HASHED);
            SetMark(mark);
            return mark.GetHash();
        }
        case MarkWord::STATE_HASHED:
            return mark.GetHash();
        default:
            LOG(FATAL, RUNTIME) << "Error on GetHashCode(): invalid state";
            return 0;
    }
}

uint32_t ObjectHeader::GetHashCodeMTMulti()
{
    ObjectHeader *current_obj = this;
    while (true) {
        auto mark = current_obj->AtomicGetMark();
        auto *thread = MTManagedThread::GetCurrent();
        ASSERT(thread != nullptr);
        [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
        VMHandle<ObjectHeader> handle_obj(thread, current_obj);

        switch (mark.GetState()) {
            case MarkWord::STATE_UNLOCKED: {
                auto hash_mark = mark.DecodeFromHash(GenerateHashCode());
                ASSERT(hash_mark.GetState() == MarkWord::STATE_HASHED);
                current_obj->AtomicSetMark(mark, hash_mark);
                break;
            }
            case MarkWord::STATE_LIGHT_LOCKED: {
                os::thread::ThreadId owner_thread_id = mark.GetThreadId();
                if (owner_thread_id == thread->GetInternalId()) {
                    Monitor::Inflate(this, thread);
                } else {
                    Monitor::InflateThinLock(thread, handle_obj);
                    current_obj = handle_obj.GetPtr();
                }
                break;
            }
            case MarkWord::STATE_HEAVY_LOCKED: {
                auto monitor_id = mark.GetMonitorId();
                auto monitor_p = MTManagedThread::GetCurrent()->GetMonitorPool()->LookupMonitor(monitor_id);
                if (monitor_p != nullptr) {
                    return GetHashCodeFromMonitor(monitor_p);
                }
                LOG(FATAL, RUNTIME) << "Error on GetHashCode(): no monitor on heavy locked state";
                break;
            }
            case MarkWord::STATE_HASHED: {
                return mark.GetHash();
            }
            default: {
                LOG(FATAL, RUNTIME) << "Error on GetHashCode(): invalid state";
            }
        }
    }
}

ObjectHeader *ObjectHeader::Clone(ObjectHeader *src)
{
    LOG_IF(src->ClassAddr<Class>()->GetManagedObject() == src, FATAL, RUNTIME) << "Can't clone a class";
    return ObjectHeader::ShallowCopy(src);
}

ObjectHeader *ObjectHeader::ShallowCopy(ObjectHeader *src)
{
    /*
     TODO(d.trubenkov):
        use bariers for possible copied reference fields
    */
    auto object_class = src->ClassAddr<Class>();
    std::size_t obj_size = src->ObjectSize();

    // AllocateObject can trigger gc, use handle for src.
    auto *thread = ManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    mem::HeapManager *heap_manager = thread->GetVM()->GetHeapManager();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<ObjectHeader> src_handle(thread, src);

    ObjectHeader *dst = nullptr;
    // Determine whether object is non-movable
    if (PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(src) == SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT) {
        dst = heap_manager->AllocateNonMovableObject(object_class, obj_size);
    } else {
        dst = heap_manager->AllocateObject(object_class, obj_size);
    }
    if (dst == nullptr) {
        return nullptr;
    }
    ASSERT(PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(src_handle.GetPtr()) ==
           PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(dst));

    Span<uint8_t> src_sp(reinterpret_cast<uint8_t *>(src_handle.GetPtr()), obj_size);
    Span<uint8_t> dst_sp(reinterpret_cast<uint8_t *>(dst), obj_size);
    constexpr const std::size_t WORD_SIZE = sizeof(uintptr_t);
    std::size_t bytes_to_copy = obj_size - ObjectHeader::ObjectHeaderSize();
    std::size_t words_to_copy = bytes_to_copy / WORD_SIZE;
    std::size_t remaining_offset = ObjectHeader::ObjectHeaderSize() + WORD_SIZE * words_to_copy;
    // copy words
    for (std::size_t i = ObjectHeader::ObjectHeaderSize(); i < remaining_offset; i += WORD_SIZE) {
        // Atomic with relaxed order reason: data race with src_handle with no synchronization or ordering constraints
        // imposed on other reads or writes
        reinterpret_cast<std::atomic<uintptr_t> *>(&dst_sp[i])
            ->store(reinterpret_cast<std::atomic<uintptr_t> *>(&src_sp[i])->load(std::memory_order_relaxed),
                    std::memory_order_relaxed);
    }
    // copy remaining bytes
    for (std::size_t i = remaining_offset; i < obj_size; i++) {
        // Atomic with relaxed order reason: data race with src_handle with no synchronization or ordering constraints
        // imposed on other reads or writes
        reinterpret_cast<std::atomic<uint8_t> *>(&dst_sp[i])
            ->store(reinterpret_cast<std::atomic<uint8_t> *>(&src_sp[i])->load(std::memory_order_relaxed),
                    std::memory_order_relaxed);
    }

    // Call barriers here.
    auto *barrier_set = thread->GetBarrierSet();
    // We don't need pre barrier here because we don't change any links inside main object
    // Post barrier
    auto gc_post_barrier_type = barrier_set->GetPostType();
    if (!mem::IsEmptyBarrier(gc_post_barrier_type)) {
        if (object_class->IsArrayClass()) {
            if (object_class->IsObjectArrayClass()) {
                barrier_set->PostBarrierArrayWrite(dst, obj_size);
            }
        } else {
            barrier_set->PostBarrierEveryObjectFieldWrite(dst, obj_size);
        }
    }
    return dst;
}

size_t ObjectHeader::ObjectSize() const
{
    auto *base_klass = ClassAddr<BaseClass>();
    if (base_klass->IsDynamicClass()) {
        auto *klass = ClassAddr<HClass>();

        if (klass->IsArray()) {
            return static_cast<const coretypes::Array *>(this)->ObjectSize(TaggedValue::TaggedTypeSize());
        }
        if (klass->IsString()) {
            LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(klass->GetSourceLang());
            return ctx.GetStringSize(this);
        }
        if (klass->IsFreeObject()) {
            return static_cast<const mem::FreeObject *>(this)->GetSize();
        }
    } else {
        auto *klass = ClassAddr<Class>();

        if (klass->IsArrayClass()) {
            return static_cast<const coretypes::Array *>(this)->ObjectSize(klass->GetComponentSize());
        }

        if (klass->IsStringClass()) {
            return static_cast<const coretypes::String *>(this)->ObjectSize();
        }

        if (klass->IsClassClass()) {
            auto cls = panda::Class::FromClassObject(const_cast<ObjectHeader *>(this));
            if (cls != nullptr) {
                return panda::Class::GetClassObjectSizeFromClass(cls);
            }
        }
    }
    return base_klass->GetObjectSize();
}

}  // namespace panda
