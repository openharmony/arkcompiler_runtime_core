/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_COROUTINES_LOCAL_STORAGE_H
#define PANDA_RUNTIME_COROUTINES_LOCAL_STORAGE_H

#include <cstddef>
#include <cstdint>
#include <array>
#include <functional>
#include "libpandabase/macros.h"
#include "runtime/coroutines/utils.h"

namespace ark {

template <class IndexType>
class StaticLocalStorage {
public:
    static constexpr size_t NUM_ENTRIES = static_cast<size_t>(IndexType::LAST_ID);
    using Finalizer = std::function<void(void *)>;
    struct Entry {
        union {
            void *ptr = nullptr;
            uintptr_t uint;
        } data;
        Finalizer finalizer = nullptr;
    };

    NO_MOVE_SEMANTIC(StaticLocalStorage);
    NO_COPY_SEMANTIC(StaticLocalStorage);

    StaticLocalStorage() = default;

    ~StaticLocalStorage()
    {
        for (auto &entry : entries_) {
            if (entry.finalizer != nullptr) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                entry.finalizer(entry.data.ptr);
            }
        }
    }

    template <IndexType IDX>
    void Set(void *pointer, const Finalizer &finalizer = nullptr)
    {
        static_assert((ToIndex(IDX) < NUM_ENTRIES), "idx should be correct");
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        entries_[ToIndex(IDX)].data.ptr = pointer;
        entries_[ToIndex(IDX)].finalizer = finalizer;
    }

    template <IndexType IDX>
    void Set(uintptr_t data)
    {
        static_assert((ToIndex(IDX) < NUM_ENTRIES), "idx should be correct");
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        entries_[ToIndex(IDX)].data.uint = data;
        entries_[ToIndex(IDX)].finalizer = nullptr;
    }

    template <IndexType IDX, class T>
    T Get()
    {
        static_assert((ToIndex(IDX) < NUM_ENTRIES), "idx should be correct");
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return reinterpret_cast<T>(entries_[ToIndex(IDX)].data.ptr);
    }

private:
    std::array<Entry, NUM_ENTRIES> entries_;
};

}  // namespace ark

#endif /* PANDA_RUNTIME_COROUTINES_LOCAL_STORAGE_H */
