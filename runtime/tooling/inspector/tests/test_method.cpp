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

#include "test_method.h"
#include "instruction_pointer.h"
#include "test_frame.h"

#include "macros.h"
#include "tooling/pt_thread.h"
#include "tooling/vreg_value.h"

namespace panda::tooling::inspector::test {
void TestMethod::Call(const std::function<void(InstructionPointer &)> &body)
{
    ASSERT(method_);

    TestFrame frame(debugger_);
    frame.SetMethod(method_);
    debugger_.GetHooks().MethodEntry(PtThread::NONE, method_);

    InstructionPointer inst(frame, client_, debugger_);
    body(inst);
    inst.Finish();

    debugger_.GetHooks().MethodExit(PtThread::NONE, method_, false, VRegValue());
}
}  // namespace panda::tooling::inspector::test
