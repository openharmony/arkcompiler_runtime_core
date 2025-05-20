/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMMON_INTERFACES_OBJECTS_BASE_TYPE_CONVERTER_H
#define COMMON_INTERFACES_OBJECTS_BASE_TYPE_CONVERTER_H

#include "objects/base_type.h"
#include "thread/thread_holder.h"

namespace panda {
// Type Converter implemented for dynamic vm.
class DynamicTypeConverterInterface {
public:
    // convert PandaType to JSTaggedValue
    virtual JSTaggedValue WrapTagged(ThreadHolder *thread, PandaType value) = 0;

    // convert JSTaggedValue to PandaType
    virtual PandaType UnWrapTagged(JSTaggedValue value) = 0;
};

// Type Converter implemented for static vm.
class StaticTypeConverterInterface {
public:
    // convert PandaType to BoxedValue
    virtual BoxedValue WrapBoxed(PandaType value) = 0;

    // convert BoxedValue to PandaType
    virtual PandaType UnWrapBoxed(BoxedValue value) = 0;
};
}  // namespace panda
#endif  // COMMON_INTERFACES_OBJECTS_BASE_TYPE_CONVERTER_H