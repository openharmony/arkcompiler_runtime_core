/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "runtime/runtime_helpers.h"
#include "runtime/jsbacktrace/backtrace.h"
#include "runtime/include/object_header-inl.h"
#include "runtime/include/runtime.h"
#include "runtime/include/stack_walker.h"
#include "runtime/include/thread.h"
#include "runtime/interpreter/runtime_interface.h"
#include "runtime/mem/object_helpers.h"

namespace ark {

void PrintStackTrace()
{
    auto thread = ManagedThread::GetCurrent();
    auto walker = StackWalker::Create(thread);
    LOG(ERROR, RUNTIME) << "====================== Stack trace begin ======================";
    for (auto stack = StackWalker::Create(thread); stack.HasFrame(); stack.NextFrame()) {
        Method *method = stack.GetMethod();
        LOG(ERROR, RUNTIME) << method->GetClass()->GetName() << "." << method->GetName().data << " at "
                            << method->GetLineNumberAndSourceFile(stack.GetBytecodePc());
    }
    LOG(ERROR, RUNTIME) << "====================== Stack trace end ======================";
}

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace panda::ecmascript;
bool ReadMemTestFunc([[maybe_unused]] void *ctx, uintptr_t addr, uintptr_t *value, bool isRead32)
{
    if (addr == 0) {
        return false;
    }
    if (isRead32) {
        *value = *(reinterpret_cast<uint32_t *>(addr));
    } else {
        *value = *(reinterpret_cast<uintptr_t *>(addr));
    }
    return true;
}

void EtsStepArkTest()
{
    auto thread = ManagedThread::GetCurrent();
    LOG(INFO, RUNTIME) << "====================== standard step begin ======================";
    auto depth = 0;
    for (auto stack = StackWalker::Create(thread); stack.HasFrame(); stack.NextFrame()) {
        LOG(INFO, RUNTIME) << "======= depth is " << depth++ << " =======";
        auto fp = reinterpret_cast<uint64_t>(stack.GetIFrame()->GetPrevFrame());
        auto pc = reinterpret_cast<uint64_t>(stack.GetIFrame()->GetInstruction());
        uint32_t bcCode = stack.GetIFrame()->GetBytecodeOffset();
        LOG(INFO, RUNTIME) << "standard fp is : " << fp;
        LOG(INFO, RUNTIME) << "standard pc is : " << pc;
        LOG(INFO, RUNTIME) << "standard bcCode is : " << bcCode;
    }
    LOG(INFO, RUNTIME) << "====================== standard step  end ======================";

    uintptr_t fp = 0;
    uintptr_t sp = 0;
    uintptr_t pc = 0;
    uintptr_t bcCode = 0;
    fp = reinterpret_cast<uintptr_t>(thread->GetCurrentFrame());
    depth = 0;
    LOG(INFO, RUNTIME) << "====================== result step begin =======================";
    while (fp != 0 && Backtrace::EtsStepArk(nullptr, &ReadMemTestFunc, &fp, &sp, &pc, &bcCode) != 0) {
        LOG(INFO, RUNTIME) << "======= depth is " << depth++ << " =======";
        LOG(INFO, RUNTIME) << "result fp is " << fp;
        LOG(INFO, RUNTIME) << "result pc is" << pc;
        LOG(INFO, RUNTIME) << "result bcCode is " << bcCode;
    }
    LOG(INFO, RUNTIME) << "====================== result step end   =======================";
}
}  // namespace ark
