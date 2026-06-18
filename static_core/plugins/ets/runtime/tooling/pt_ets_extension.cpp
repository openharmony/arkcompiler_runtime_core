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

#include "plugins/ets/runtime/tooling/pt_ets_extension.h"

#ifdef PANDA_ETS_INTEROP_JS
#include "runtime/execution/coroutines/coroutine.h"
#include "runtime/include/tooling/buffer_serializer.h"
#include "plugins/ets/runtime/interop_js/tooling/internal_api.h"
#endif

namespace ark::ets {

// clang-format off
bool PtEtsExtension::CollectHybridStackFrames([[maybe_unused]] const std::function<
    void(const void *frame, bool isStaticFrame)> &callback)
{
#ifdef PANDA_ETS_INTEROP_JS
    auto thread = ManagedThread::GetCurrent();
    if (thread == nullptr || !ark::Coroutine::MutatorIsCoroutine(thread)) {
        return false;
    }

    // Use ForEachFrameInUnionStack to traverse the hybrid stack
    ark::ets::interop::js::ForEachFrameInUnionStack(callback);
    return true;
#else
    return false;
#endif
}
// clang-format on

size_t PtEtsExtension::GetDynamicFrameInfo(const void *frame, void *buffer, size_t bufferSize, uintptr_t *fileId,
                                           uint32_t *bcOffset)
{
#ifdef PANDA_ETS_INTEROP_JS
    arkplatform::HybridFrameInfo hybridFrameInfo;
    bool succeed = ark::ets::interop::js::GetDynamicFrameInfo(frame, hybridFrameInfo);
    if (!succeed) {
        return 0;
    }

    if (fileId != nullptr) {
        *fileId = hybridFrameInfo.fileId;
    }
    if (bcOffset != nullptr) {
        *bcOffset = hybridFrameInfo.bcOffset;
    }

    ark::tooling::BufferSerializer::PluginFrameData frameData {
        hybridFrameInfo.GetFunctionName(), hybridFrameInfo.GetModuleName(), hybridFrameInfo.GetUrl(),
        hybridFrameInfo.lineNumber, hybridFrameInfo.columnNumber};
    return ark::tooling::BufferSerializer::SerializePluginFrameData(frameData, static_cast<uint8_t *>(buffer),
                                                                    bufferSize);
#else
    (void)frame;
    (void)buffer;
    (void)bufferSize;
    (void)fileId;
    (void)bcOffset;
    return 0;
#endif
}

bool PtEtsExtension::IsSupportHybridStack()
{
#ifdef PANDA_ETS_INTEROP_JS
    return ark::ets::interop::js::IsHybridStackEnabled();
#else
    return false;
#endif
}

}  // namespace ark::ets
