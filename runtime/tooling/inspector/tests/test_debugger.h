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

#ifndef PANDA_TOOLING_INSPECTOR_TEST_TEST_DEBUGGER_H
#define PANDA_TOOLING_INSPECTOR_TEST_TEST_DEBUGGER_H

#include "macros.h"
#include "tooling/debugger.h"
#include "tooling/debug_interface.h"
#include "tooling/pt_location.h"
#include "utils/list.h"

#include "gmock/gmock.h"

#include <unordered_set>

namespace panda::tooling::inspector::test {
class TestFrame;

class TestDebugger : public DebugInterface {
public:
    std::optional<Error> RegisterHooks(PtHooks *hooks) override
    {
        hooks_ = hooks;
        return {};
    }
    std::optional<Error> UnregisterHooks() override
    {
        hooks_ = nullptr;
        return {};
    }
    PtHooks &GetHooks()
    {
        ASSERT(hooks_ != nullptr);
        return *hooks_;
    }

    void PushFrame(TestFrame &frame);
    void PopFrame()
    {
        callStack_.PopFront();
    }

    Expected<std::unique_ptr<PtFrame>, Error> GetCurrentFrame(PtThread thread) const override;
    std::optional<Error> EnumerateFrames(PtThread thread, std::function<bool(const PtFrame &)> function) const override;
    std::optional<Error> NotifyFramePop(PtThread thread, uint32_t frameDepth) const override;

    void HandleBreakpoint(const PtLocation &location) const;
    std::optional<Error> SetBreakpoint(const PtLocation &location) override;
    std::optional<Error> RemoveBreakpoint(const PtLocation &location) override;

    MOCK_METHOD(std::optional<Error>, EnableAllGlobalHook, (), (override));
    MOCK_METHOD(std::optional<Error>, DisableAllGlobalHook, (), (override));
    MOCK_METHOD(std::optional<Error>, SetNotification, (PtThread, bool, PtHookType), (override));
    MOCK_METHOD(std::optional<Error>, SuspendThread, (PtThread), (const override));
    MOCK_METHOD(std::optional<Error>, ResumeThread, (PtThread), (const override));
    MOCK_METHOD(PtLangExt *, GetLangExtension, (), (const override));
    MOCK_METHOD((Expected<PtMethod, Error>), GetPtMethod, (const PtLocation &), (const override));
    MOCK_METHOD(std::optional<Error>, GetThreadList, (PandaVector<PtThread> *), (const override));
    MOCK_METHOD(std::optional<Error>, SetVariable, (PtThread, uint32_t, int32_t, const PtValue &), (const override));
    MOCK_METHOD(std::optional<Error>, GetVariable, (PtThread, uint32_t, int32_t, PtValue *), (const override));
    MOCK_METHOD(std::optional<Error>, GetProperty, (PtObject, PtProperty, PtValue *), (const override));
    MOCK_METHOD(std::optional<Error>, SetProperty, (PtObject, PtProperty, const PtValue &), (const override));
    MOCK_METHOD(std::optional<Error>, EvaluateExpression, (PtThread, uint32_t, ExpressionWrapper, PtValue *),
                (const override));
    MOCK_METHOD(std::optional<Error>, RetransformClasses, (int, const PtClass *), (const override));
    MOCK_METHOD(std::optional<Error>, RedefineClasses, (int, const PandaClassDefinition *), (const override));
    MOCK_METHOD(std::optional<Error>, GetThreadInfo, (PtThread, ThreadInfo *), (const override));
    MOCK_METHOD(std::optional<Error>, RestartFrame, (PtThread, uint32_t), (const override));
    MOCK_METHOD(std::optional<Error>, SetAsyncCallStackDepth, (uint32_t), (const override));
    MOCK_METHOD(std::optional<Error>, AwaitPromise, (PtObject, PtValue *), (const override));
    MOCK_METHOD(std::optional<Error>, CallFunctionOn, (PtObject, PtMethod, const PandaVector<PtValue> &, PtValue *),
                (const override));
    MOCK_METHOD(std::optional<Error>, GetProperties, (uint32_t *, char ***), (const override));
    MOCK_METHOD(std::optional<Error>, GetThisVariableByFrame, (PtThread, uint32_t, ObjectHeader **), (override));
    MOCK_METHOD(std::optional<Error>, SetPropertyAccessWatch, (BaseClass *, PtProperty), (override));
    MOCK_METHOD(std::optional<Error>, ClearPropertyAccessWatch, (BaseClass *, PtProperty), (override));
    MOCK_METHOD(std::optional<Error>, SetPropertyModificationWatch, (BaseClass *, PtProperty), (override));
    MOCK_METHOD(std::optional<Error>, ClearPropertyModificationWatch, (BaseClass *, PtProperty), (override));
    MOCK_METHOD(std::optional<Error>, SetPropertyAccessWatch, (PtClass, PtProperty), (override));
    MOCK_METHOD(std::optional<Error>, ClearPropertyAccessWatch, (PtClass, PtProperty), (override));
    MOCK_METHOD(std::optional<Error>, SetPropertyModificationWatch, (PtClass, PtProperty), (override));
    MOCK_METHOD(std::optional<Error>, GetThisVariableByFrame, (PtThread, uint32_t, PtValue *), (override));
    MOCK_METHOD(std::optional<Error>, ClearPropertyModificationWatch, (PtClass, PtProperty), (override));

private:
    PtHooks *hooks_ {nullptr};
    mutable List<TestFrame> callStack_;
    std::unordered_set<PtLocation, HashLocation> breakpoints_;
};
}  // namespace panda::tooling::inspector::test

#endif  // PANDA_TOOLING_INSPECTOR_TEST_TEST_DEBUGGER_H
