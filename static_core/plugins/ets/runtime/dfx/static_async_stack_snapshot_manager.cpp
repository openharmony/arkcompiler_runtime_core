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

#include "plugins/ets/runtime/dfx/static_async_stack_snapshot_manager.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

#include "plugins/ets/runtime/dfx/ets_async_stack_snapshot_handle.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/types/ets_async_stack_snapshot.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "runtime/execution/job.h"
#include "runtime/include/stack_walker.h"
#include "runtime/include/runtime.h"
#include "libarkbase/utils/utf.h"

namespace ark::ets {

namespace {

struct CapturedFrame {
    std::string functionName;
    std::string pandaFile;
    uint32_t methodId = 0U;
    uint32_t bytecodeOffset = 0U;
    uint64_t nativePc = 0U;
};

void DiscardOptionalFailure(EtsExecutionContext *executionCtx, bool hadPendingException)
{
    auto *mt = executionCtx->GetMT();
    if (hadPendingException || !mt->HasPendingException()) {
        return;
    }

    auto *exception = EtsObject::FromCoreType(mt->GetException());
    if (exception->IsInstanceOf(PlatformTypes(executionCtx)->coreOutOfMemoryError)) {
        mt->ClearException();
    }
}

bool IsPromiseInternalFrame(EtsExecutionContext *executionCtx, Method *method)
{
    return method->GetClass() == PlatformTypes(executionCtx)->corePromise->GetRuntimeClass();
}

const char *GetPromiseSubmissionDescription(EtsExecutionContext *executionCtx, Method *method)
{
    if (method->GetClass() != PlatformTypes(executionCtx)->corePromise->GetRuntimeClass()) {
        return nullptr;
    }

    auto *methodName = utf::Mutf8AsCString(method->GetName().data);
    if (std::strcmp(methodName, "then") == 0) {
        return "promise.then";
    }
    if (std::strcmp(methodName, "catch") == 0) {
        return "promise.catch";
    }
    if (std::strcmp(methodName, "finally") == 0) {
        return "promise.finally";
    }
    return nullptr;
}

std::vector<CapturedFrame> CaptureFrames(EtsExecutionContext *executionCtx, uint32_t frameLimit, bool skipTopFrame)
{
    std::vector<CapturedFrame> frames;
    bool topFrameSkipped = false;
    for (auto stack = StackWalker::Create(executionCtx->GetMT()); stack.HasFrame() && frames.size() < frameLimit;
         stack.NextFrame()) {
        auto *method = stack.GetMethod();
        // Promise-internal frames are implementation details and are never captured. skipTopFrame skips the first
        // captured frame, matching call-site captures where the async helper is not part of the logical history.
        if (method == nullptr || IsPromiseInternalFrame(executionCtx, method)) {
            continue;
        }

        if (skipTopFrame && !topFrameSkipped) {
            topFrameSkipped = true;
            continue;
        }

        const auto *pandaFile = method->GetPandaFile();
        const auto methodName = method->GetFullName();
        frames.push_back(CapturedFrame {methodName.c_str(),  // NOLINT(readability-redundant-string-cstr)
                                        pandaFile == nullptr ? "" : pandaFile->GetFilename(),
                                        method->GetFileId().GetOffset(), static_cast<uint32_t>(stack.GetBytecodePc()),
                                        static_cast<uint64_t>(stack.GetNativePc())});
    }
    return frames;
}

EtsAsyncStackSnapshot *GetLocalParentSnapshot(EtsExecutionContext *executionCtx)
{
    auto *handle = Job::GetCurrentAsyncDebuggerStack();
    if (handle == nullptr || handle->GetSnapshotTypeId() != EtsAsyncStackSnapshotHandle::SnapshotTypeId() ||
        handle->GetOpaqueOwner() != executionCtx->GetPandaVM()) {
        return nullptr;
    }

    return static_cast<EtsAsyncStackSnapshot *>(handle->GetOpaqueSnapshot());
}

EtsAsyncStackSourceIdentity *CreateSourceIdentity(EtsExecutionContext *executionCtx, const CapturedFrame &frame)
{
    auto *sourceIdentity = EtsAsyncStackSourceIdentity::Create(executionCtx);
    if (UNLIKELY(sourceIdentity == nullptr)) {
        return nullptr;
    }
    EtsHandle<EtsAsyncStackSourceIdentity> sourceIdentityHandle(executionCtx, sourceIdentity);

    auto *pandaFile = EtsString::CreateFromMUtf8(frame.pandaFile.c_str());
    if (UNLIKELY(pandaFile == nullptr)) {
        return nullptr;
    }
    EtsHandle<EtsString> pandaFileHandle(executionCtx, pandaFile);
    sourceIdentityHandle->SetPandaFile(executionCtx, pandaFileHandle.GetPtr());
    sourceIdentityHandle->SetMethodId(static_cast<EtsLong>(frame.methodId));
    sourceIdentityHandle->SetBytecodeOffset(static_cast<EtsInt>(frame.bytecodeOffset));
    sourceIdentityHandle->SetNativePc(static_cast<EtsLong>(frame.nativePc));
    return sourceIdentityHandle.GetPtr();
}

EtsAsyncStackFrame *CreateAsyncFrame(EtsExecutionContext *executionCtx, const CapturedFrame &frame)
{
    auto *sourceIdentity = CreateSourceIdentity(executionCtx, frame);
    if (UNLIKELY(sourceIdentity == nullptr)) {
        return nullptr;
    }
    EtsHandle<EtsAsyncStackSourceIdentity> sourceIdentityHandle(executionCtx, sourceIdentity);

    auto *asyncFrame = EtsAsyncStackFrame::Create(executionCtx);
    if (UNLIKELY(asyncFrame == nullptr)) {
        return nullptr;
    }
    EtsHandle<EtsAsyncStackFrame> asyncFrameHandle(executionCtx, asyncFrame);

    auto *functionName = EtsString::CreateFromMUtf8(frame.functionName.c_str());
    if (UNLIKELY(functionName == nullptr)) {
        return nullptr;
    }
    EtsHandle<EtsString> functionNameHandle(executionCtx, functionName);
    asyncFrameHandle->SetFunctionName(executionCtx, functionNameHandle.GetPtr());
    asyncFrameHandle->SetSourceIdentity(executionCtx, sourceIdentityHandle.GetPtr());
    return asyncFrameHandle.GetPtr();
}

EtsHandle<EtsObjectArray> CreateFramesArray(EtsExecutionContext *executionCtx, const std::vector<CapturedFrame> &frames)
{
    EtsHandle<EtsObjectArray> emptyHandle;
    auto *framesArray = EtsObjectArray::Create(PlatformTypes(executionCtx)->coreAsyncStackFrame, frames.size());
    if (UNLIKELY(framesArray == nullptr)) {
        return emptyHandle;
    }
    EtsHandle<EtsObjectArray> framesArrayHandle(executionCtx, framesArray);

    for (size_t idx = 0; idx < frames.size(); ++idx) {
        auto *asyncFrame = CreateAsyncFrame(executionCtx, frames[idx]);
        if (UNLIKELY(asyncFrame == nullptr)) {
            return emptyHandle;
        }
        EtsHandle<EtsAsyncStackFrame> asyncFrameHandle(executionCtx, asyncFrame);
        framesArrayHandle->Set(static_cast<uint32_t>(idx), asyncFrameHandle.GetPtr());
    }
    return framesArrayHandle;
}

EtsAsyncStackSegment *CreateSegment(EtsExecutionContext *executionCtx, const char *description,
                                    const AsyncDebuggerConfigSnapshot &config,
                                    EtsHandle<EtsObjectArray> &framesArrayHandle)
{
    auto *descriptionString = EtsString::CreateFromMUtf8(description);
    if (UNLIKELY(descriptionString == nullptr)) {
        return nullptr;
    }
    EtsHandle<EtsString> descriptionHandle(executionCtx, descriptionString);

    auto *segment = EtsAsyncStackSegment::Create(executionCtx);
    if (UNLIKELY(segment == nullptr)) {
        return nullptr;
    }
    EtsHandle<EtsAsyncStackSegment> segmentHandle(executionCtx, segment);
    segmentHandle->SetDescription(executionCtx, descriptionHandle.GetPtr());
    segmentHandle->SetGeneration(static_cast<EtsLong>(config.generation));
    segmentHandle->SetFrames(executionCtx, framesArrayHandle.GetPtr());
    return segmentHandle.GetPtr();
}

size_t GetInheritedSegmentCount(EtsExecutionContext *executionCtx, EtsHandle<EtsAsyncStackSnapshot> &parentHandle,
                                const AsyncDebuggerConfigSnapshot &config)
{
    if (parentHandle.GetPtr() == nullptr || parentHandle->GetGeneration() != static_cast<EtsLong>(config.generation)) {
        return 0U;
    }

    auto *parentSegments = parentHandle->GetSegments(executionCtx);
    if (parentSegments == nullptr) {
        return 0U;
    }
    return std::min<size_t>(parentSegments->GetLength(), config.maxAsyncDepth - 1U);
}

EtsAsyncStackSnapshot *CreateSnapshot(EtsExecutionContext *executionCtx, EtsHandle<EtsAsyncStackSegment> &segmentHandle,
                                      EtsHandle<EtsAsyncStackSnapshot> &parentHandle,
                                      const AsyncDebuggerConfigSnapshot &config)
{
    const size_t inheritedSegments = GetInheritedSegmentCount(executionCtx, parentHandle, config);
    auto *segmentsArray =
        EtsObjectArray::Create(PlatformTypes(executionCtx)->coreAsyncStackSegment, inheritedSegments + 1U);
    if (UNLIKELY(segmentsArray == nullptr)) {
        return nullptr;
    }
    EtsHandle<EtsObjectArray> segmentsArrayHandle(executionCtx, segmentsArray);
    segmentsArrayHandle->Set(0U, segmentHandle.GetPtr());

    if (parentHandle.GetPtr() != nullptr && inheritedSegments != 0U) {
        auto *parentSegments = parentHandle->GetSegments(executionCtx);
        for (size_t idx = 0; idx < inheritedSegments; ++idx) {
            segmentsArrayHandle->Set(static_cast<uint32_t>(idx + 1U),
                                     EtsAsyncStackSegment::FromEtsObject(parentSegments->Get(idx)));
        }
    }

    auto *snapshot = EtsAsyncStackSnapshot::Create(executionCtx);
    if (UNLIKELY(snapshot == nullptr)) {
        return nullptr;
    }
    EtsHandle<EtsAsyncStackSnapshot> snapshotHandle(executionCtx, snapshot);
    snapshotHandle->SetGeneration(static_cast<EtsLong>(config.generation));
    snapshotHandle->SetSegments(executionCtx, segmentsArrayHandle.GetPtr());
    return snapshotHandle.GetPtr();
}

}  // namespace

const char *StaticAsyncStackSnapshotManager::ClassifyPromiseSubmission(EtsExecutionContext *executionCtx)
{
    ASSERT(executionCtx != nullptr);
    for (auto stack = StackWalker::Create(executionCtx->GetMT()); stack.HasFrame(); stack.NextFrame()) {
        auto *method = stack.GetMethod();
        if (method != nullptr) {
            if (auto *description = GetPromiseSubmissionDescription(executionCtx, method); description != nullptr) {
                return description;
            }
        }
    }
    return nullptr;
}

EtsAsyncStackSnapshot *StaticAsyncStackSnapshotManager::Capture(EtsExecutionContext *executionCtx,
                                                                const char *description, bool skipTopFrame)
{
    ASSERT(executionCtx != nullptr);
    ASSERT(description != nullptr);

    [[maybe_unused]] auto *mt = executionCtx->GetMT();
    auto *runtime = Runtime::GetCurrent();
    const auto config = executionCtx->GetPandaVM()->GetAsyncDebuggerConfig();
    if (runtime == nullptr || !runtime->IsDebugMode() || !config.IsCaptureEnabled()) {
        return nullptr;
    }

    // Collect runtime identities before any managed allocation. This keeps StackWalker iteration
    // and GC allocation mutually exclusive.
    auto frames = CaptureFrames(executionCtx, config.maxFramesPerSegment, skipTopFrame);

    EtsHandleScope scope(executionCtx);

    auto framesArrayHandle = CreateFramesArray(executionCtx, frames);
    if (UNLIKELY(framesArrayHandle.GetPtr() == nullptr)) {
        ASSERT(mt->HasPendingException());
        return nullptr;
    }

    auto *segment = CreateSegment(executionCtx, description, config, framesArrayHandle);
    if (UNLIKELY(segment == nullptr)) {
        ASSERT(mt->HasPendingException());
        return nullptr;
    }
    EtsHandle<EtsAsyncStackSegment> segmentHandle(executionCtx, segment);

    EtsHandle<EtsAsyncStackSnapshot> parentHandle(executionCtx, GetLocalParentSnapshot(executionCtx));
    auto *snapshot = CreateSnapshot(executionCtx, segmentHandle, parentHandle, config);
    if (UNLIKELY(snapshot == nullptr)) {
        ASSERT(mt->HasPendingException());
        return nullptr;
    }
    EtsHandle<EtsAsyncStackSnapshot> snapshotHandle(executionCtx, snapshot);
    return snapshotHandle.GetPtr();
}

EtsAsyncStackSnapshot *StaticAsyncStackSnapshotManager::CaptureManaged(EtsExecutionContext *executionCtx,
                                                                       const char *description, bool skipTopFrame)
{
    const bool hadPendingException = executionCtx->GetMT()->HasPendingException();
    auto *snapshot = Capture(executionCtx, description, skipTopFrame);
    if (UNLIKELY(snapshot == nullptr)) {
        DiscardOptionalFailure(executionCtx, hadPendingException);
    }
    return snapshot;
}

AsyncStackSnapshotHandlePtr StaticAsyncStackSnapshotManager::CreateHandle(EtsExecutionContext *executionCtx,
                                                                          EtsAsyncStackSnapshot *snapshot)
{
    if (snapshot == nullptr) {
        return AsyncStackSnapshotHandlePtr {};
    }

    const bool hadPendingException = executionCtx->GetMT()->HasPendingException();
    auto handle = EtsAsyncStackSnapshotHandle::Create(executionCtx->GetPandaVM(), snapshot);
    if (UNLIKELY(handle == nullptr)) {
        DiscardOptionalFailure(executionCtx, hadPendingException);
    }
    return handle;
}

AsyncStackSnapshotHandlePtr StaticAsyncStackSnapshotManager::CaptureHandle(EtsExecutionContext *executionCtx,
                                                                           const char *description, bool skipTopFrame)
{
    auto *snapshot = CaptureManaged(executionCtx, description, skipTopFrame);
    if (snapshot == nullptr) {
        return AsyncStackSnapshotHandlePtr {};
    }
    return CreateHandle(executionCtx, snapshot);
}

void StaticAsyncStackSnapshotManager::DiscardOptionalSnapshotFailure(EtsExecutionContext *executionCtx,
                                                                     bool hadPendingException)
{
    DiscardOptionalFailure(executionCtx, hadPendingException);
}

}  // namespace ark::ets
