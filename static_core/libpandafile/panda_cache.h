/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef LIBPANDAFILE_PANDA_CACHE_H_
#define LIBPANDAFILE_PANDA_CACHE_H_

#include "file.h"
#include "os/mutex.h"
#include "libpandabase/utils/math_helpers.h"

#include <atomic>
#include <vector>

namespace ark {

class Method;
class Field;
class Class;

namespace panda_file {

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
class PandaCache {
public:
    struct MethodCachePair {
        File::EntityId id;
        Method *ptr {nullptr};
    };

    struct FieldCachePair {
        File::EntityId id;
        Field *ptr {nullptr};
    };

    struct ClassCachePair {
        File::EntityId id;
        Class *ptr {nullptr};
    };

    PandaCache()
        : methodCacheSize_(DEFAULT_METHOD_CACHE_SIZE),
          fieldCacheSize_(DEFAULT_FIELD_CACHE_SIZE),
          classCacheSize_(DEFAULT_CLASS_CACHE_SIZE)
    {
        methodCache_.resize(methodCacheSize_, MethodCachePair());
        fieldCache_.resize(fieldCacheSize_, FieldCachePair());
        classCache_.resize(classCacheSize_, ClassCachePair());
    }

    ~PandaCache() = default;

    inline uint32_t GetMethodIndex(File::EntityId id) const
    {
        return ark::helpers::math::PowerOfTwoTableSlot(id.GetOffset(), methodCacheSize_);
    }

    inline uint32_t GetFieldIndex(File::EntityId id) const
    {
        // lowest one or two bits is very likely same between different fields
        return ark::helpers::math::PowerOfTwoTableSlot(id.GetOffset(), fieldCacheSize_, 2U);
    }

    inline uint32_t GetClassIndex(File::EntityId id) const
    {
        return ark::helpers::math::PowerOfTwoTableSlot(id.GetOffset(), classCacheSize_);
    }

    inline Method *GetMethodFromCache(File::EntityId id) const
    {
        // Emulator target doesn't support atomic operations with 128bit structures like MethodCachePair.
        // Compiler __atomic_load call which is not implemented in emulator target.
#ifndef PANDA_TARGET_EMULATOR
        uint32_t index = GetMethodIndex(id);
        auto *pairPtr =
            reinterpret_cast<std::atomic<MethodCachePair> *>(reinterpret_cast<uintptr_t>(&(methodCache_[index])));
        // Atomic with acquire order reason: fixes a data race with method_cache_
        auto pair = pairPtr->load(std::memory_order_acquire);
        TSAN_ANNOTATE_HAPPENS_AFTER(pairPtr);
        if (pair.id == id) {
            return pair.ptr;
        }
#endif
        return nullptr;
    }

    inline void SetMethodCache(File::EntityId id, Method *method)
    {
        // Emulator target doesn't support atomic operations with 128bit structures like MethodCachePair.
        // Compiler __atomic_load call which is not implemented in emulator target.
#ifndef PANDA_TARGET_EMULATOR
        MethodCachePair pair;
        pair.id = id;
        pair.ptr = method;
        uint32_t index = GetMethodIndex(id);
        auto *pairPtr =
            reinterpret_cast<std::atomic<MethodCachePair> *>(reinterpret_cast<uintptr_t>(&(methodCache_[index])));
        TSAN_ANNOTATE_HAPPENS_BEFORE(pairPtr);
        // Atomic with release order reason: fixes a data race with method_cache_
        pairPtr->store(pair, std::memory_order_release);
#endif
    }

    inline Field *GetFieldFromCache(File::EntityId id) const
    {
        // Emulator target doesn't support atomic operations with 128bit structures like FieldCachePair.
        // Compiler __atomic_load call which is not implemented in emulator target.
#ifndef PANDA_TARGET_EMULATOR
        uint32_t index = GetFieldIndex(id);
        auto *pairPtr =
            reinterpret_cast<std::atomic<FieldCachePair> *>(reinterpret_cast<uintptr_t>(&(fieldCache_[index])));
        // Atomic with acquire order reason: fixes a data race with field_cache_
        auto pair = pairPtr->load(std::memory_order_acquire);
        TSAN_ANNOTATE_HAPPENS_AFTER(pairPtr);
        if (pair.id == id) {
            return pair.ptr;
        }
#endif
        return nullptr;
    }

    inline void SetFieldCache(File::EntityId id, Field *field)
    {
        // Emulator target doesn't support atomic operations with 128bit structures like FieldCachePair.
        // Compiler __atomic_load call which is not implemented in emulator target.
#ifndef PANDA_TARGET_EMULATOR
        uint32_t index = GetFieldIndex(id);
        auto *pairPtr =
            reinterpret_cast<std::atomic<FieldCachePair> *>(reinterpret_cast<uintptr_t>(&(fieldCache_[index])));
        FieldCachePair pair;
        pair.id = id;
        pair.ptr = field;
        TSAN_ANNOTATE_HAPPENS_BEFORE(pairPtr);
        // Atomic with release order reason: fixes a data race with field_cache_
        pairPtr->store(pair, std::memory_order_release);
#endif
    }

    inline Class *GetClassFromCache(File::EntityId id) const
    {
        // Emulator target doesn't support atomic operations with 128bit structures like ClassCachePair.
        // Compiler __atomic_load call which is not implemented in emulator target.
#ifndef PANDA_TARGET_EMULATOR
        uint32_t index = GetClassIndex(id);
        auto *pairPtr =
            reinterpret_cast<std::atomic<ClassCachePair> *>(reinterpret_cast<uintptr_t>(&(classCache_[index])));
        // Atomic with acquire order reason: fixes a data race with class_cache_
        auto pair = pairPtr->load(std::memory_order_acquire);
        TSAN_ANNOTATE_HAPPENS_AFTER(pairPtr);
        if (pair.id == id) {
            return pair.ptr;
        }
#endif
        return nullptr;
    }

    inline void SetClassCache(File::EntityId id, Class *clazz)
    {
        // Emulator target doesn't support atomic operations with 128bit structures like ClassCachePair.
        // Compiler __atomic_load call which is not implemented in emulator target.
#ifndef PANDA_TARGET_EMULATOR
        ClassCachePair pair;
        pair.id = id;
        pair.ptr = clazz;
        uint32_t index = GetClassIndex(id);
        auto *pairPtr =
            reinterpret_cast<std::atomic<ClassCachePair> *>(reinterpret_cast<uintptr_t>(&(classCache_[index])));
        TSAN_ANNOTATE_HAPPENS_BEFORE(pairPtr);
        // Atomic with release order reason: fixes a data race with class_cache_
        pairPtr->store(pair, std::memory_order_release);
#endif
    }

    inline void Clear()
    {
        methodCache_.clear();
        fieldCache_.clear();
        classCache_.clear();

        methodCache_.resize(methodCacheSize_, MethodCachePair());
        fieldCache_.resize(fieldCacheSize_, FieldCachePair());
        classCache_.resize(classCacheSize_, ClassCachePair());
    }

    template <class Callback>
    bool EnumerateCachedClasses(const Callback &cb)
    {
        for (uint32_t i = 0; i < classCacheSize_; i++) {
            auto *pairPtr =
                reinterpret_cast<std::atomic<ClassCachePair> *>(reinterpret_cast<uintptr_t>(&(classCache_[i])));
            // Atomic with acquire order reason: fixes a data race with class_cache_
            auto pair = pairPtr->load(std::memory_order_acquire);
            TSAN_ANNOTATE_HAPPENS_AFTER(pairPtr);
            if (pair.ptr != nullptr) {
                if (!cb(pair.ptr)) {
                    return false;
                }
            }
        }
        return true;
    }

private:
    static constexpr uint32_t DEFAULT_FIELD_CACHE_SIZE = 1024U;
    static constexpr uint32_t DEFAULT_METHOD_CACHE_SIZE = 1024U;
    static constexpr uint32_t DEFAULT_CLASS_CACHE_SIZE = 1024U;
    static_assert(ark::helpers::math::IsPowerOfTwo(DEFAULT_FIELD_CACHE_SIZE));
    static_assert(ark::helpers::math::IsPowerOfTwo(DEFAULT_METHOD_CACHE_SIZE));
    static_assert(ark::helpers::math::IsPowerOfTwo(DEFAULT_CLASS_CACHE_SIZE));

    const uint32_t methodCacheSize_;
    const uint32_t fieldCacheSize_;
    const uint32_t classCacheSize_;

    std::vector<MethodCachePair> methodCache_;
    std::vector<FieldCachePair> fieldCache_;
    std::vector<ClassCachePair> classCache_;
};

}  // namespace panda_file
}  // namespace ark

#endif
