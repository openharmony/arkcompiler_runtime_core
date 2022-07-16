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

#include "instruction_pointer.h"

#include "../inspector.h"
#include "client.h"
#include "json_object_matcher.h"

#include "bytecode_instruction.h"
#include "macros.h"
#include "tooling/pt_location.h"
#include "tooling/pt_thread.h"
#include "utils/json_builder.h"
#include "utils/json_parser.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace panda::tooling::inspector::test {
InstructionPointer::~InstructionPointer()
{
    EXPECT_EQ(instruction_.GetAddress(), instruction_.GetFrom()) << "InstructionPointer not finished stepping";
}

void InstructionPointer::ContinueTo(std::string_view scriptId, size_t lineNumber)
{
    client_.Call(
        "Debugger.continueToLocation",
        [&](auto &params) {
            params.AddProperty("location", [&](JsonObjectBuilder &location) {
                location.AddProperty("scriptId", scriptId);
                location.AddProperty("lineNumber", lineNumber);
            });
        },
        // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks) due to bug in clang-tidy #3553 (gtest repo)
        [](auto &result) { EXPECT_THAT(result, JsonProperties()); });

    Step();
}

void InstructionPointer::Finish()
{
    if (unset_) {
        Reset();
    }

    while (instruction_.IsValid()) {
        Step();
    }
}

void InstructionPointer::Resume()
{
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks) due to bug in clang-tidy #3553 (gtest repo)
    client_.Call("Debugger.resume", [](const JsonObject &result) { EXPECT_THAT(result, JsonProperties()); });
    Step();
}

void InstructionPointer::Step()
{
    if (unset_) {
        Reset();
    }

    ASSERT(instruction_.IsValid());

    PtLocation location(GetMethod()->GetPandaFile()->GetFilename().c_str(), GetMethod()->GetFileId(),
                        instruction_.GetOffset());

    frame_.SetBytecodeOffset(instruction_.GetOffset());

    debugger_.GetHooks().SingleStep(PtThread::NONE, GetMethod(), location);
    debugger_.HandleBreakpoint(location);

    instruction_ = instruction_.GetNext();
}

void InstructionPointer::StepInto()
{
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks) due to bug in clang-tidy #3553 (gtest repo)
    client_.Call("Debugger.stepInto", [](const JsonObject &result) { EXPECT_THAT(result, JsonProperties()); });
    Step();
}

void InstructionPointer::StepOut()
{
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks) due to bug in clang-tidy #3553 (gtest repo)
    client_.Call("Debugger.stepOut", [](const JsonObject &result) { EXPECT_THAT(result, JsonProperties()); });
    Step();
}

void InstructionPointer::StepOver()
{
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks) due to bug in clang-tidy #3553 (gtest repo)
    client_.Call("Debugger.stepOver", [](const JsonObject &result) { EXPECT_THAT(result, JsonProperties()); });
    Step();
}

void InstructionPointer::Reset()
{
    ASSERT(GetMethod());
    instruction_ = BytecodeInstructionSafe(GetMethod()->GetInstructions(), GetMethod()->GetInstructions(),
                                           GetMethod()->GetInstructions() + GetMethod()->GetCodeSize() - 1);
    frame_.SetBytecodeOffset(0);
    unset_ = false;
}
}  // namespace panda::tooling::inspector::test
