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
#ifndef COMPILER_OPTIMIZER_ANALYSIS_COUNTABLE_LOOP_PARSER_H_
#define COMPILER_OPTIMIZER_ANALYSIS_COUNTABLE_LOOP_PARSER_H_

#include "optimizer/ir/inst.h"

namespace panda::compiler {
class Loop;

/**
 * Example of code
 *  ---------------
 * for (init(a); if_imm(compare(a,test)); update(a)) {...}
 */
struct CountableLoopInfo {
    Inst *if_imm;
    Inst *init;
    Inst *test;
    Inst *update;
    Inst *index;
    uint64_t const_step;
    ConditionCode normalized_cc;  // cc between `update` and `test`
};

/**
 * Helper class to check if loop is countable and to get its parameters
 */
class CountableLoopParser {
public:
    explicit CountableLoopParser(const Loop &loop) : loop_(loop) {}

    NO_MOVE_SEMANTIC(CountableLoopParser);
    NO_COPY_SEMANTIC(CountableLoopParser);
    ~CountableLoopParser() = default;

    std::optional<CountableLoopInfo> Parse();

private:
    Inst *SetIndexAndRetrunConstInst();

private:
    const Loop &loop_;
    CountableLoopInfo loop_info_ {};
    bool is_head_loop_exit_ = false;
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_ANALYSIS_COUNTABLE_LOOP_PARSER_H_
