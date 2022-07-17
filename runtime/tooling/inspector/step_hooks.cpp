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

#include "step_hooks.h"
#include "inspector.h"
#include "error.h"
#include "json_property.h"
#include "server.h"

#include "bytecode_instruction-inl.h"
#include "optimizer/ir_builder/inst_builder.h"
#include "tooling/debug_interface.h"
#include "tooling/pt_location.h"
#include "utils/json_parser.h"
#include "utils/logger.h"
#include "utils/string_helpers.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <utility>

using namespace std::placeholders;  // NOLINT(google-build-using-namespace)
using panda::helpers::string::ParseInt;

namespace panda::tooling::inspector {
StepHooks::StepHooks(Inspector &inspector) : inspector_(inspector)
{
    inspector.GetServer().OnCall("Debugger.continueToLocation", std::bind(&StepHooks::ContinueToLocation, this, _2));
    inspector.GetServer().OnCall("Debugger.stepInto", std::bind(&StepHooks::SetStep, this, STEP_INTO));
    inspector.GetServer().OnCall("Debugger.stepOut", std::bind(&StepHooks::SetStep, this, STEP_OUT));
    inspector.GetServer().OnCall("Debugger.stepOver", std::bind(&StepHooks::SetStep, this, STEP_OVER));
    inspector.GetServer().OnCall("Runtime.runIfWaitingForDebugger", std::bind(&StepHooks::SetBreakOnStart, this));

    inspector.OnPause(std::bind(&StepHooks::OnPause, this));
}

void StepHooks::AddLocation(const PtLocation &location)
{
    if (state_->locations.count(location) != 0) {
        return;
    }

    if (auto error = inspector_.GetDebugger().SetBreakpoint(location)) {
        if (error->GetType() == Error::Type::BREAKPOINT_ALREADY_EXISTS) {
            state_->locations[location] = true;
        } else {
            HandleError(std::move(error));
        }
    } else {
        state_->locations[location] = false;
    }
}

void StepHooks::Breakpoint(PtThread thread, Method * /* method */, const PtLocation &location)
{
    if (state_ && state_->thread == thread && state_->locations.count(location) != 0) {
        inspector_.SetPause(thread, "Step");
    }
}

void StepHooks::ContinueToLocation(const JsonObject &params)
{
    const std::string *scriptIdString;
    if (!GetPropertyOrLog<JsonObject::StringT>(scriptIdString, params, "location", "scriptId")) {
        return;
    }

    int scriptId;
    if (!ParseInt(*scriptIdString, &scriptId, 0)) {
        LOG(INFO, DEBUGGER) << "Invalid script id: " << *scriptIdString;
        return;
    }

    size_t lineNumber;
    if (!GetPropertyOrLog<JsonObject::NumT>(lineNumber, params, "location", "lineNumber")) {
        return;
    }

    auto sourceFile = inspector_.GetSourceManager().GetSourceFile(scriptId);
    if (sourceFile == nullptr) {
        return;
    }

    state_ = State {};
    state_->stepKind = CONTINUE_TO;
    state_->thread = inspector_.Resume();

    sourceFile->EnumerateLocations(lineNumber, [&](auto location) {
        AddLocation(location);
        return true;
    });
}

void StepHooks::FramePop(PtThread thread, Method * /* method */, bool /* wasPoppedByException */)
{
    if (state_ && state_->thread == thread) {
        state_->pauseOnStep = true;
    }
}

void StepHooks::MethodEntry(PtThread thread, Method * /* method */)
{
    if (!state_ || state_->thread != thread || std::exchange(state_->methodEntered, true)) {
        return;
    }

    switch (state_->stepKind) {
        case BREAK_ON_START:
        case CONTINUE_TO:
        case STEP_OUT:
            break;

        case STEP_INTO:
            state_->pauseOnStep = true;
            break;

        case STEP_OVER:
            HandleError(inspector_.GetDebugger().NotifyFramePop(thread, 0));
            break;
    }
}

void StepHooks::OnPause()
{
    if (!state_) {
        return;
    }

    for (auto &[location, existingBreakpoint] : state_->locations) {
        if (!existingBreakpoint) {
            HandleError(inspector_.GetDebugger().RemoveBreakpoint(location));
        }
    }

    state_.reset();
}

void StepHooks::SetBreakOnStart()
{
    inspector_.Resume();

    state_ = State {};
    state_->stepKind = BREAK_ON_START;
}

void StepHooks::SetStep(StepKind stepKind)
{
    auto thread = inspector_.Resume();

    state_ = State {};
    state_->stepKind = stepKind;
    state_->thread = thread;

    if (stepKind == STEP_OUT) {
        HandleError(inspector_.GetDebugger().NotifyFramePop(thread, 0));
        return;
    }

    auto frame = inspector_.GetDebugger().GetCurrentFrame(thread);
    if (!frame) {
        HandleError(frame.Error());
        return;
    }

    auto method = frame.Value()->GetMethod();
    uint32_t bytecodeOffset = frame.Value()->GetBytecodeOffset();

    auto &table =
        inspector_.GetSourceManager().GetDebugInfo(method->GetPandaFile()).GetLineNumberTable(method->GetFileId());
    auto nextLineIter = std::upper_bound(table.begin(), table.end(), bytecodeOffset,
                                         [](auto offset, auto &entry) { return offset < entry.offset; });
    uint32_t nextLineBytecodeOffset = nextLineIter != table.end() ? nextLineIter->offset : ~0U;

    while (bytecodeOffset < std::min(nextLineBytecodeOffset, method->GetCodeSize())) {
        BytecodeInstruction inst(method->GetInstructions() + bytecodeOffset);

        if (inst.HasFlag(BytecodeInstruction::Flags::JUMP)) {
            AddLocation(PtLocation(method->GetPandaFile()->GetFilename().c_str(), method->GetFileId(),
                                   bytecodeOffset + compiler::InstBuilder::GetInstructionJumpOffset(&inst)));

            if (!inst.HasFlag(BytecodeInstruction::Flags::CONDITIONAL)) {
                break;
            }
        }

        bytecodeOffset += inst.GetSize();
    }

    if (bytecodeOffset == nextLineBytecodeOffset) {
        AddLocation(
            PtLocation(method->GetPandaFile()->GetFilename().c_str(), method->GetFileId(), nextLineBytecodeOffset));
    } else {
        HandleError(inspector_.GetDebugger().NotifyFramePop(thread, 0));
    }
}

void StepHooks::SingleStep(PtThread thread, Method * /* method */, const PtLocation & /* location */)
{
    if (!state_) {
        return;
    }

    if (state_->stepKind == BREAK_ON_START) {
        inspector_.SetPause(thread, "Break on start");
    } else if (state_->thread == thread && state_->pauseOnStep) {
        inspector_.SetPause(thread, "Step");
    }
}

void StepHooks::VmInitialization(PtThread thread)
{
    // Wait for a debugger to attach.
    inspector_.SetPause(thread, {});
}
}  // namespace panda::tooling::inspector
