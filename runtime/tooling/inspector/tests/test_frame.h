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

#ifndef PANDA_TOOLING_INSPECTOR_TEST_TEST_FRAME_H
#define PANDA_TOOLING_INSPECTOR_TEST_TEST_FRAME_H

#include "test_debugger.h"

#include "macros.h"
#include "method.h"
#include "tooling/debug_interface.h"
#include "tooling/pt_thread.h"
#include "utils/list.h"

#include <cstdint>

namespace panda::tooling::inspector::test {
class TestFrame : public PtFrame, public ListNode {
public:
    explicit TestFrame(TestDebugger &debugger) : debugger_(&debugger)
    {
        debugger.PushFrame(*this);
    }

    TestFrame(const TestFrame &other)  // NOLINT(bugprone-copy-constructor-init)
        : PtFrame(),                   // NOLINT(readability-redundant-member-init)
          ListNode(),                  // NOLINT(readability-redundant-member-init)
          method_(other.method_),
          bytecodeOffset_(other.bytecodeOffset_)
    {
    }

    TestFrame &operator=(const TestFrame &other)
    {
        if (&other == this) {
            return *this;
        }

        debugger_ = nullptr;
        method_ = other.method_;
        bytecodeOffset_ = other.bytecodeOffset_;
        notifyPop_ = false;
        return *this;
    }

    NO_MOVE_SEMANTIC(TestFrame);

    ~TestFrame() override
    {
        if (notifyPop_) {
            ASSERT(debugger_ != nullptr);
            debugger_->GetHooks().FramePop(PtThread::NONE, method_, false);
        }

        if (debugger_ != nullptr) {
            debugger_->PopFrame();
        }
    }

    bool IsInterpreterFrame() const override
    {
        return true;
    }

    void SetMethod(Method *method)
    {
        method_ = method;
    }

    Method *GetMethod() const override
    {
        return method_;
    }

    uint64_t GetVReg(size_t /* i */) const override
    {
        return 0;
    }

    size_t GetVRegNum() const override
    {
        return 0;
    }

    uint64_t GetArgument(size_t /* i */) const override
    {
        return 0;
    }

    size_t GetArgumentNum() const override
    {
        return 0;
    }

    uint64_t GetAccumulator() const override
    {
        return 0;
    }

    panda_file::File::EntityId GetMethodId() const override
    {
        return method_->GetFileId();
    }

    void SetBytecodeOffset(uint32_t bytecodeOffset)
    {
        bytecodeOffset_ = bytecodeOffset;
    }

    uint32_t GetBytecodeOffset() const override
    {
        return bytecodeOffset_;
    }

    std::string GetPandaFile() const override
    {
        return method_->GetPandaFile()->GetFilename();
    }

    uint32_t GetFrameId() const override
    {
        return reinterpret_cast<uintptr_t>(this);
    }

    void SetNotifyPop()
    {
        ASSERT(debugger_ != nullptr);
        notifyPop_ = true;
    }

private:
    TestDebugger *debugger_ {nullptr};
    Method *method_ {nullptr};
    uint32_t bytecodeOffset_ {0};
    bool notifyPop_ {false};
};
}  // namespace panda::tooling::inspector::test

#endif  // PANDA_TOOLING_INSPECTOR_TEST_TEST_FRAME_H
