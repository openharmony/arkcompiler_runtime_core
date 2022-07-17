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

#include "test_debugger.h"
#include "test_frame.h"

#include "macros.h"
#include "tooling/debug_interface.h"
#include "tooling/pt_thread.h"
#include "utils/expected.h"

#include <algorithm>
#include <iterator>
#include <memory>

namespace panda::tooling::inspector::test {
void TestDebugger::PushFrame(TestFrame &frame)
{
    return callStack_.PushFront(frame);
}

Expected<std::unique_ptr<PtFrame>, Error> TestDebugger::GetCurrentFrame(PtThread /* thread */) const
{
    if (callStack_.Empty()) {
        return Unexpected(Error(Error::Type::NO_MORE_FRAMES, "No current frame"));
    }

    return {std::make_unique<TestFrame>(callStack_.Front())};
}

std::optional<Error> TestDebugger::EnumerateFrames(PtThread /* thread */,
                                                   std::function<bool(const PtFrame &)> function) const
{
    std::all_of(callStack_.begin(), callStack_.end(), function);
    return {};
}

std::optional<Error> TestDebugger::NotifyFramePop(PtThread /* thread */, uint32_t frameDepth) const
{
    std::next(callStack_.begin(), frameDepth)->SetNotifyPop();
    return {};
}

void TestDebugger::HandleBreakpoint(const PtLocation &location) const
{
    if (breakpoints_.count(location) == 0) {
        return;
    }

    ASSERT(hooks_ != nullptr && !callStack_.Empty());
    hooks_->Breakpoint(PtThread::NONE, callStack_.Front().GetMethod(), location);
}

std::optional<Error> TestDebugger::SetBreakpoint(const PtLocation &location)
{
    if (!breakpoints_.insert(location).second) {
        return Error(Error::Type::BREAKPOINT_ALREADY_EXISTS, "Breakpoint already exists");
    }

    return {};
}

std::optional<Error> TestDebugger::RemoveBreakpoint(const PtLocation &location)
{
    if (breakpoints_.erase(location) == 0) {
        return Error(Error::Type::BREAKPOINT_NOT_FOUND, "Breakpoint not found");
    }

    return {};
}
}  // namespace panda::tooling::inspector::test
