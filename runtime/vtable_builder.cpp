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

#include "runtime/include/vtable_builder.h"
#include "runtime/include/class_linker.h"

namespace panda {

// check max-specific method or not according to JVM specs chap5.4.3.3
// we have to guarantee that while we are iterating itable, the child interface has to be accessed before father
// interface. interface without inheritance has no limit.
bool IsMaxSpecificMethod(const Class *iface, const Method &method, size_t startindex, const ITable &itable)
{
    for (size_t j = startindex; j < itable.Size(); j++) {
        auto current_iface = itable[j].GetInterface();
        if (iface->IsAssignableFrom(current_iface)) {
            for (auto &curmethod : current_iface->GetVirtualMethods()) {
                if (method.GetName() == curmethod.GetName() && method.GetProto() == curmethod.GetProto()) {
                    ASSERT(curmethod.IsAbstract());
                    return false;
                }
            }
        }
    }
    return true;
}

}  // namespace panda
