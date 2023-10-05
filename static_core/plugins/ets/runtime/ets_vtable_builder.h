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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_VTABLE_BUILDER_H_
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_VTABLE_BUILDER_H_

#include <cstddef>
#include <utility>

#include "runtime/include/vtable_builder-inl.h"

namespace panda::ets {

struct EtsVTableSearchBySignature {
    bool operator()(const MethodInfo &info1, const MethodInfo &info2) const
    {
        if (info1.IsEqualByNameAndSignature(info2)) {
            return true;
        }
        // NOTE: the best way is to check subtyping of return types.
        // If true, check IsEqualByNameAndSignature with replaced return types.
        // But now no way to check subtyping or casting for methods
        if (info1.GetName() != info2.GetName()) {
            return false;
        }
        if (((info1.IsAbstract() ^ info2.IsAbstract()) != 0) && (info1.GetReturnType() == info2.GetReturnType()) &&
            (info1.GetSourceLang() == info2.GetSourceLang())) {
            return true;
        }
        return false;
    }
};

struct EtsVTableOverridePred {
    std::pair<bool, size_t> operator()(const MethodInfo &info1, const MethodInfo &info2) const
    {
        // Do not override already overrided default methods
        if (info2.IsInterfaceMethod() && !info1.IsCopied()) {
            // Default iface override should take inheritance info consideration.
            if (info1.IsInterfaceMethod()) {
                ASSERT(info1.GetMethod() != nullptr);
                ASSERT(info2.GetMethod() != nullptr);
                Class *cls1 = info1.GetMethod()->GetClass();
                Class *cls2 = info2.GetMethod()->GetClass();
                if (cls2->IsAssignableFrom(cls1)) {
                    return std::pair<bool, size_t>(false, MethodInfo::INVALID_METHOD_IDX);
                }
                return std::pair<bool, size_t>(true, info1.GetIndex());
            }
            return std::pair<bool, size_t>(false, MethodInfo::INVALID_METHOD_IDX);
        }

        if (info1.IsPublic() || info1.IsProtected()) {
            return std::pair<bool, size_t>(true, MethodInfo::INVALID_METHOD_IDX);
        }

        if (info1.IsPrivate()) {
            return std::pair<bool, size_t>(false, MethodInfo::INVALID_METHOD_IDX);
        }

        return std::pair<bool, size_t>(IsInSamePackage(info1, info2), MethodInfo::INVALID_METHOD_IDX);
    }

    bool IsInSamePackage(const MethodInfo &info1, const MethodInfo &info2) const;
};

using EtsVTableBuilder = VTableBuilderImpl<EtsVTableSearchBySignature, EtsVTableOverridePred>;

}  // namespace panda::ets

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_ETS_ITABLE_BUILDER_H_
