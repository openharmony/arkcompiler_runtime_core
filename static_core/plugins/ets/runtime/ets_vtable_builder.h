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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_VTABLE_BUILDER_H_
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_VTABLE_BUILDER_H_

#include <cstddef>
#include <utility>

#include "runtime/include/vtable_builder_variance-inl.h"
#include "plugins/ets/runtime/ets_vm.h"

namespace ark::ets {

bool ETSProtoIsOverriddenBy(Method::ProtoId const &base, Method::ProtoId const &derv);

struct EtsVTableCompatibleSignatures {
    bool operator()(const Method::ProtoId &base, const Method::ProtoId &derv) const
    {
        return ETSProtoIsOverriddenBy(base, derv);
    }
};

struct EtsVTableOverridePred {
    bool operator()(const MethodInfo *base, const MethodInfo *derv) const
    {
        if (base->IsPublic() || base->IsProtected()) {
            return true;
        }

        if (base->IsPrivate()) {
            return false;
        }

        return IsInSamePackage(*base, *derv);
    }

    bool IsInSamePackage(const MethodInfo &info1, const MethodInfo &info2) const;
};

using EtsVTableBuilder = VarianceVTableBuilder<EtsVTableCompatibleSignatures, EtsVTableOverridePred>;

}  // namespace ark::ets

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_ETS_ITABLE_BUILDER_H_
