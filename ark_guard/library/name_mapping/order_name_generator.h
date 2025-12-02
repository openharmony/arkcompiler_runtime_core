/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef ARK_GUARD_NAME_MAPPING_ORDER_NAME_GENERATOR_H
#define ARK_GUARD_NAME_MAPPING_ORDER_NAME_GENERATOR_H

#include <cstdint>
#include <unordered_set>

#include "name_generator.h"

namespace ark::guard {

/**
 * @brief OrderNameGenerator
 */
class OrderNameGenerator final : public NameGenerator {
public:
    /**
     * @brief default constructor, enabling object creation without parameters
     */
    OrderNameGenerator() = default;

    /**
     * @brief constructor, initializes the usedNameList_
     * @param usedNameList used name list
     */
    explicit OrderNameGenerator(std::unordered_set<std::string> &&usedNameList) : NameGenerator(std::move(usedNameList))
    {
    }

    /**
     * @brief Generate ordered name, for example: a, b, c, ..., a1, b1, c1, ...
     * @return ordered name
     */
    std::string GenerateName() override;

private:
    uint32_t charIndex_ = 0;

    uint32_t loopNumber_ = 0;
};
}  // namespace ark::guard

#endif  // ARK_GUARD_NAME_MAPPING_ORDER_NAME_GENERATOR_H
