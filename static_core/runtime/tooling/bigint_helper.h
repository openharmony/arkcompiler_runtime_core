/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_TOOLING_BIGINT_HELPER_H
#define PANDA_RUNTIME_TOOLING_BIGINT_HELPER_H

#include <cstdint>
#include <string>
#include <vector>
#include "runtime/include/mem/panda_containers.h"

namespace ark::tooling {

std::vector<int> AddDecimalArrays(const std::vector<int> &lhs, const std::vector<int> &rhs);

std::vector<int> MultiplyDecimalArrayByBigInt(std::vector<int> lhs, const std::vector<int> &rhs);

std::string DecimalArrayToString(const std::vector<int> &digits);

std::string BigIntBytesToDecimalString(const PandaVector<uint32_t> &bytes, int sign);

}  // namespace ark::tooling
#endif
