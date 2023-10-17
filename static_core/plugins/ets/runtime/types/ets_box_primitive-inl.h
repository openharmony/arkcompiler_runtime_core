/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/types/ets_box_primitive.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"

namespace panda::ets {
template <typename T>
EtsBoxPrimitive<T> *EtsBoxPrimitive<T>::Create(EtsCoroutine *coro, T value)
{
    auto *ext = coro->GetPandaVM()->GetClassLinker()->GetEtsClassLinkerExtension();
    Class *box_class = nullptr;
    if constexpr (std::is_same<T, EtsBoolean>::value) {
        box_class = ext->GetBoxBooleanClass();
    } else if constexpr (std::is_same<T, EtsByte>::value) {
        box_class = ext->GetBoxByteClass();
    } else if constexpr (std::is_same<T, EtsChar>::value) {
        box_class = ext->GetBoxCharClass();
    } else if constexpr (std::is_same<T, EtsShort>::value) {
        box_class = ext->GetBoxShortClass();
    } else if constexpr (std::is_same<T, EtsInt>::value) {
        box_class = ext->GetBoxIntClass();
    } else if constexpr (std::is_same<T, EtsLong>::value) {
        box_class = ext->GetBoxLongClass();
    } else if constexpr (std::is_same<T, EtsFloat>::value) {
        box_class = ext->GetBoxFloatClass();
    } else if constexpr (std::is_same<T, EtsDouble>::value) {
        box_class = ext->GetBoxDoubleClass();
    }
    auto *instance = reinterpret_cast<EtsBoxPrimitive<T> *>(ObjectHeader::Create(coro, box_class));
    instance->SetValue(value);
    return instance;
}
}  // namespace panda::ets
