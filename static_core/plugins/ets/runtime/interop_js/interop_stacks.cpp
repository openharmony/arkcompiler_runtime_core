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

#include "plugins/ets/runtime/interop_js/interop_stacks.h"
#include <string>
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "runtime/include/stack_walker.h"
#include "runtime/interpreter/frame.h"
#include "runtime/include/method.h"
#include "runtime/tooling/debugger.h"
#include "plugins/ets/runtime/interop_js/interop_error.h"

namespace ark::ets::interop::js {

InteropCallStack::InteropCallStack()
{
    void *pool = ark::os::mem::MapRWAnonymousWithAlignmentRaw(POOL_SIZE, ark::os::mem::GetPageSize());
    if (pool == nullptr) {
        InteropFatal(INTEROP_INTEROP_CALL_STACK_ALLOCATION_FAILED, "InteropCallStack alloc failed");
    }
    startAddr_ = reinterpret_cast<Record *>(pool);
    endAddr_ = reinterpret_cast<Record *>(ToUintPtr(pool) + POOL_SIZE);
    curAddr_ = startAddr_;
}

void *InteropCallStack::GetStaticTopFrame() const
{
    auto *coro = EtsCoroutine::GetCurrent();
    if (UNLIKELY(coro == nullptr)) {
        return nullptr;
    }
    auto stack = StackWalker::Create(coro);
    if (!stack.HasFrame()) {
        return nullptr;
    }
    // NOLINTNEXTLINE
    return stack.IsCFrame() ? static_cast<void *>(&stack.GetCFrame()) : static_cast<void *>(stack.GetIFrame());
}

void *InteropCallStack::GetDynamicTopFrameSP() const
{
    auto *ctx = interop::js::InteropCtx::Current();
    if (UNLIKELY(ctx == nullptr)) {
        return nullptr;
    }
    auto *ecmaInterface = ctx->GetECMAInterface();
    if (UNLIKELY(ecmaInterface == nullptr)) {
        return nullptr;
    }
    return ecmaInterface->GetTopFrameSPFromDynamic();
}

bool InteropCallStack::ForEachDynamicFrame(void *currFrameSP, void *toFrameSP,
                                           const std::function<void(const void *frame)> &cb) const
{
    auto *ctx = interop::js::InteropCtx::Current();
    if (UNLIKELY(ctx == nullptr)) {
        return false;
    }
    auto *ecmaInterface = ctx->GetECMAInterface();
    if (UNLIKELY(ecmaInterface == nullptr)) {
        return false;
    }
    return ecmaInterface->ForEachDynamicFrame(currFrameSP, toFrameSP, cb);
}

bool InteropCallStack::ForEachStaticFrame(StackWalker *stack, void *toFrame,
                                          const std::function<void(const void *frame)> &cb) const
{
    for (; stack->HasFrame(); stack->NextFrame()) {
        void *currFrame =
            stack->IsCFrame() ? static_cast<void *>(&stack->GetCFrame()) : static_cast<void *>(stack->GetIFrame());
        if (currFrame == toFrame) {
            break;
        }
        Method *method = stack->GetMethod();
        Frame *frame {};
        if (!stack->IsCFrame()) {
            frame = stack->GetIFrame();
        }

        tooling::PtDebugFrame debugFrame = tooling::PtDebugFrame(method, frame);
        cb(&debugFrame);
    }
    return true;
}

bool InteropCallStack::ForEachInteropFrame(const std::function<void(const void *frame, bool isStaticFrame)> &callback)
{
    auto *coro = EtsCoroutine::GetCurrent();
    if (UNLIKELY(coro == nullptr)) {
        return false;
    }
    // Get the current top stack and traverse the stack between the current node and the recorded node.
    StackWalker staticStack = StackWalker::Create(coro);
    void *dynamicFrameSP = GetDynamicTopFrameSP();
    void *toFrame = nullptr;

    int frameSize = static_cast<int>(GetRecords().size());

    for (int fIdx = frameSize - 1; fIdx >= 0; fIdx--) {
        toFrame = GetRecords()[fIdx].frame;
        bool nextIsStatic = GetRecords()[fIdx].isStaticFrame;
        if (nextIsStatic) {
            // Parse static frames.
            if (!ForEachStaticFrame(&staticStack, toFrame, [&callback](const void *frame) { callback(frame, true); })) {
                return false;
            }
        } else {
            // Parse dynamic frames.
            if (!ForEachDynamicFrame(dynamicFrameSP, toFrame,
                                     [&callback](const void *frame) { callback(frame, false); })) {
                return false;
            }
            dynamicFrameSP = toFrame;
        }
    }

    // Parse all remaining static frames ans dynamic frames.
    if (frameSize > 0 && GetRecords()[0U].isStaticFrame) {
        return ForEachDynamicFrame(dynamicFrameSP, nullptr,
                                   [&callback](const void *frame) { callback(frame, false); }) &&
               ForEachStaticFrame(&staticStack, nullptr, [&callback](const void *frame) { callback(frame, true); });
    }

    return ForEachStaticFrame(&staticStack, nullptr, [&callback](const void *frame) { callback(frame, true); }) &&
           ForEachDynamicFrame(dynamicFrameSP, nullptr, [&callback](const void *frame) { callback(frame, false); });
}

}  // namespace ark::ets::interop::js
