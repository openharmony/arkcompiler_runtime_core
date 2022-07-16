/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef PANDA_TOOLING_INSPECTOR_TEST_INSTRUCTION_POINTER_H
#define PANDA_TOOLING_INSPECTOR_TEST_INSTRUCTION_POINTER_H

#include "test_frame.h"

#include "bytecode_instruction.h"
#include "macros.h"

#include <cstddef>
#include <string_view>

namespace panda::tooling::inspector::test {
class Client;
class TestDebugger;

class InstructionPointer {
public:
    InstructionPointer(TestFrame &frame, Client &client, TestDebugger &debugger)
        : frame_(frame), client_(client), debugger_(debugger)
    {
    }

    NO_COPY_SEMANTIC(InstructionPointer);
    DEFAULT_MOVE_CTOR(InstructionPointer)
    NO_MOVE_OPERATOR(InstructionPointer);

    ~InstructionPointer();

    Method *GetMethod() const
    {
        return frame_.GetMethod();
    }

    void ContinueTo(std::string_view scriptId, size_t lineNumber);
    void Finish();
    void Resume();
    void Step();
    void StepInto();
    void StepOut();
    void StepOver();

private:
    void Reset();

    TestFrame &frame_;
    Client &client_;
    TestDebugger &debugger_;
    BytecodeInstructionSafe instruction_;
    bool unset_ {true};
};
}  // namespace panda::tooling::inspector::test

#endif  // PANDA_TOOLING_INSPECTOR_TEST_INSTRUCTION_POINTER_H
