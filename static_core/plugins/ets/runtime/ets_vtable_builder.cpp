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

#include "plugins/ets/runtime/ets_vtable_builder.h"

#include "runtime/include/class_linker.h"

namespace ark::ets {

bool EtsVTableOverridePred::IsInSamePackage(const MethodInfo &info1, const MethodInfo &info2) const
{
    if (info1.GetLoadContext() != info2.GetLoadContext()) {
        return false;
    }

    auto *desc1 = info1.GetClassName();
    auto *desc2 = info2.GetClassName();

    while (ClassHelper::IsArrayDescriptor(desc1)) {
        desc1 = ClassHelper::GetComponentDescriptor(desc1);
    }

    while (ClassHelper::IsArrayDescriptor(desc2)) {
        desc2 = ClassHelper::GetComponentDescriptor(desc2);
    }

    Span sp1(desc1, 1);
    Span sp2(desc2, 1);
    while (sp1[0] == sp2[0] && sp1[0] != 0 && sp2[0] != 0) {
        sp1 = Span(sp1.cend(), 1);
        sp2 = Span(sp2.cend(), 1);
    }

    bool isSamePackage = true;
    while (sp1[0] != 0 && isSamePackage) {
        isSamePackage = sp1[0] != '/';
        sp1 = Span(sp1.cend(), 1);
    }

    while (sp2[0] != 0 && isSamePackage) {
        isSamePackage = sp2[0] != '/';
        sp2 = Span(sp2.cend(), 1);
    }

    return isSamePackage;
}

}  // namespace ark::ets
