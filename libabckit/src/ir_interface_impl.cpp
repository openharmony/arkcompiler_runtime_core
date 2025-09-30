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

#include "libabckit/src/ir_impl.h"
#include <cassert>

std::string AbckitIrInterface::GetMethodIdByOffset(uint32_t offset) const
{
    auto it = methods.find(offset);
    ASSERT(it != methods.cend());

    return std::string(it->second);
}

std::string AbckitIrInterface::GetStringIdByOffset(uint32_t offset) const
{
    auto it = strings.find(offset);
    ASSERT(it != strings.cend());

    return std::string(it->second);
}

std::string AbckitIrInterface::GetLiteralArrayIdByOffset(uint32_t offset) const
{
    auto it = literalarrays.find(offset);
    ASSERT(it != literalarrays.cend());

    return std::string(it->second);
}

std::string AbckitIrInterface::GetTypeIdByOffset(uint32_t offset) const
{
    auto it = classes.find(offset);
    ASSERT(it != classes.cend());

    return std::string(it->second);
}

std::string AbckitIrInterface::GetFieldIdByOffset(uint32_t offset) const
{
    auto it = fields.find(offset);
    ASSERT(it != fields.cend());

    return std::string(it->second);
}
