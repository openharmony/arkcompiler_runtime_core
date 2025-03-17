/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_STUBS_H
#define PANDA_PLUGINS_ETS_RUNTIME_STUBS_H

#include <cstdint>

namespace ark::ets {

constexpr static uint64_t METHOD_FLAG_MASK = 0x00000001;

class EtsCoroutine;
class EtsObject;
class EtsClass;
class EtsString;

// Generic comparator for ets reference types
template <bool IS_STRICT = false>
inline bool EtsReferenceEquals(EtsCoroutine *coro, EtsObject *ref1, EtsObject *ref2);

inline bool EtsReferenceNullish(EtsCoroutine *coro, EtsObject *ref);

// Comparison slowpath for value-typed references
bool EtsValueTypedEquals(EtsCoroutine *coro, EtsObject *obj1, EtsObject *obj2);

// Obtain owner class of method in ets frames
inline EtsClass *GetMethodOwnerClassInFrames(EtsCoroutine *coro, uint32_t depth);

// Compute typeof
EtsString *EtsGetTypeof(EtsCoroutine *coro, EtsObject *obj);

bool EtsGetIstrue(EtsCoroutine *coro, EtsObject *obj);

template <bool IS_GETTER>
inline void LookUpException(ark::Class *klass, Field *rawField);

inline void LookUpException(ark::Class *klass, ark::Method *rawMethod);

template <bool IS_GETTER = true>
inline Field *GetFieldByName(InterpreterCache::Entry *entry, ark::Method *method, Field *rawField,
                             const uint8_t *address, ark::Class *klass);

template <panda_file::Type::TypeId FIELD_TYPE, bool IS_GETTER>
inline ark::Method *GetAccessorByName(InterpreterCache::Entry *entry, ark::Method *method, Field *rawField,
                                      const uint8_t *address, ark::Class *klass);

inline Field *LookupFieldByName(panda_file::File::StringData name, const ark::Class *klass);

template <panda_file::Type::TypeId FIELD_TYPE>
inline ark::Method *LookupGetterByName(panda_file::File::StringData name, const ark::Class *klass);

template <panda_file::Type::TypeId FIELD_TYPE>
inline ark::Method *LookupSetterByName(panda_file::File::StringData name, const ark::Class *klass);

struct ETSStubCacheInfo {
public:
    ETSStubCacheInfo(InterpreterCache::Entry *entry, Method *instMethod, const uint8_t *instAddress)
        : entry_(entry), instMethod_(instMethod), instAddress_(instAddress)
    {
    }

    void *GetItem() const
    {
        return entry_->item;
    }

    void UpdateItem(void *value) const
    {
        auto mUint = reinterpret_cast<uint64_t>(value);
        *entry_ = {instAddress_, instMethod_, reinterpret_cast<Method *>(mUint | METHOD_FLAG_MASK)};
    }

private:
    InterpreterCache::Entry *entry_ {};
    Method *instMethod_ {};
    const uint8_t *instAddress_ {};
};

Method *GetMethodByName(EtsCoroutine *coro, ETSStubCacheInfo const &cache, Method *rawMethod, ark::Class *klass);
}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_STUBS_H
