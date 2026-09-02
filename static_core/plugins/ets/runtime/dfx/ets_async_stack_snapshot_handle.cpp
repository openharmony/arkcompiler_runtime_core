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

#include "plugins/ets/runtime/dfx/ets_async_stack_snapshot_handle.h"

#include <new>
#include <optional>
#include <string>
#include <utility>

#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_async_stack_snapshot.h"
#include "runtime/mem/refstorage/reference.h"

namespace ark::ets {

namespace {

std::string ConvertToString(EtsString *value)
{
    if (value == nullptr) {
        return {};
    }

    // GetUtf8 converts both compressed MUTF-8 and UTF-16 string representations.
    auto utf8 = value->GetUtf8();
    return {utf8.data(), utf8.size()};
}

std::optional<AsyncStackFrameView> CreateFrameView(EtsExecutionContext *executionCtx, EtsAsyncStackFrame *frameObject)
{
    if (frameObject == nullptr) {
        return std::nullopt;
    }

    EtsHandle<EtsAsyncStackFrame> frameHandle(executionCtx, frameObject);
    auto *sourceIdentityObject = frameHandle->GetSourceIdentity(executionCtx);
    if (sourceIdentityObject == nullptr) {
        return std::nullopt;
    }
    EtsHandle<EtsAsyncStackSourceIdentity> sourceIdentityHandle(executionCtx, sourceIdentityObject);

    AsyncStackFrameView frame;
    frame.functionName = ConvertToString(frameHandle->GetFunctionName(executionCtx));
    frame.pandaFile = ConvertToString(sourceIdentityHandle->GetPandaFile(executionCtx));
    frame.methodId = static_cast<uint64_t>(sourceIdentityHandle->GetMethodId());
    frame.bytecodeOffset = static_cast<uint32_t>(sourceIdentityHandle->GetBytecodeOffset());
    frame.nativePc = static_cast<uint64_t>(sourceIdentityHandle->GetNativePc());
    return frame;
}

std::optional<AsyncStackSegmentView> CreateSegmentView(EtsExecutionContext *executionCtx,
                                                       EtsAsyncStackSegment *segmentObject)
{
    if (segmentObject == nullptr) {
        return std::nullopt;
    }

    EtsHandle<EtsAsyncStackSegment> segmentHandle(executionCtx, segmentObject);
    auto *framesObject = segmentHandle->GetFrames(executionCtx);
    if (framesObject == nullptr) {
        return std::nullopt;
    }
    EtsHandle<EtsObjectArray> framesHandle(executionCtx, framesObject);

    AsyncStackSegmentView segment;
    segment.description = ConvertToString(segmentHandle->GetDescription(executionCtx));
    segment.generation = static_cast<uint64_t>(segmentHandle->GetGeneration());
    segment.frames.reserve(framesHandle->GetLength());
    for (uint32_t frameIdx = 0; frameIdx < framesHandle->GetLength(); ++frameIdx) {
        auto frame = CreateFrameView(executionCtx, EtsAsyncStackFrame::FromEtsObject(framesHandle->Get(frameIdx)));
        if (!frame.has_value()) {
            return std::nullopt;
        }
        segment.frames.push_back(std::move(*frame));
    }
    return segment;
}

}  // namespace

AsyncStackSnapshotHandlePtr EtsAsyncStackSnapshotHandle::Create(PandaEtsVM *vm, EtsAsyncStackSnapshot *snapshot)
{
    return Create(vm, snapshot, false);
}

AsyncStackSnapshotHandlePtr EtsAsyncStackSnapshotHandle::CreateWithAllocationFailureForTest(
    PandaEtsVM *vm, EtsAsyncStackSnapshot *snapshot)
{
    return Create(vm, snapshot, true);
}

AsyncStackSnapshotHandlePtr EtsAsyncStackSnapshotHandle::Create(PandaEtsVM *vm, EtsAsyncStackSnapshot *snapshot,
                                                                bool failNativeAllocation)
{
    ASSERT(vm != nullptr);
    ASSERT(snapshot != nullptr);
    auto *reference = vm->GetGlobalObjectStorage()->Add(snapshot->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
    if (reference == nullptr) {
        return nullptr;
    }

    AsyncStackSnapshotHandlePtr handle;
    if (!failNativeAllocation) {
        handle = AsyncStackSnapshotHandlePtr(new (std::nothrow) EtsAsyncStackSnapshotHandle(vm, reference));
    }
    if (handle == nullptr) {
        vm->GetGlobalObjectStorage()->Remove(reference);
    }
    return handle;
}

const void *EtsAsyncStackSnapshotHandle::SnapshotTypeId()
{
    static constexpr char TYPE_ID = 0;
    return &TYPE_ID;
}

EtsAsyncStackSnapshotHandle::EtsAsyncStackSnapshotHandle(PandaEtsVM *vm, mem::Reference *reference)
    : vm_(vm), reference_(reference)
{
}

EtsAsyncStackSnapshotHandle::~EtsAsyncStackSnapshotHandle()
{
    vm_->GetGlobalObjectStorage()->Remove(reference_);
    reference_ = nullptr;
}

AsyncStackSnapshotHandlePtr EtsAsyncStackSnapshotHandle::Clone() const
{
    return Create(vm_, GetSnapshot());
}

std::unique_ptr<AsyncStackSnapshotView> EtsAsyncStackSnapshotHandle::CreateSnapshotView() const
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    if (executionCtx == nullptr || executionCtx->GetPandaVM() != vm_) {
        return nullptr;
    }

    EtsHandleScope scope(executionCtx);
    EtsHandle<EtsAsyncStackSnapshot> snapshotHandle(executionCtx, GetSnapshot());
    auto *segmentsObject = snapshotHandle->GetSegments(executionCtx);
    if (segmentsObject == nullptr) {
        return nullptr;
    }
    EtsHandle<EtsObjectArray> segmentsHandle(executionCtx, segmentsObject);

    auto view = std::make_unique<AsyncStackSnapshotView>();
    view->generation = static_cast<uint64_t>(snapshotHandle->GetGeneration());
    view->segments.reserve(segmentsHandle->GetLength());

    for (uint32_t segmentIdx = 0; segmentIdx < segmentsHandle->GetLength(); ++segmentIdx) {
        auto segment =
            CreateSegmentView(executionCtx, EtsAsyncStackSegment::FromEtsObject(segmentsHandle->Get(segmentIdx)));
        if (!segment.has_value()) {
            return nullptr;
        }
        view->segments.push_back(std::move(*segment));
    }

    return view;
}

const void *EtsAsyncStackSnapshotHandle::GetSnapshotTypeId() const noexcept
{
    return SnapshotTypeId();
}

void *EtsAsyncStackSnapshotHandle::GetOpaqueSnapshot() const noexcept
{
    return GetSnapshot();
}

void *EtsAsyncStackSnapshotHandle::GetOpaqueOwner() const noexcept
{
    return vm_;
}

EtsAsyncStackSnapshot *EtsAsyncStackSnapshotHandle::GetSnapshot() const noexcept
{
    ASSERT(reference_ != nullptr);
    auto *object = vm_->GetGlobalObjectStorage()->Get(reference_);
    return EtsAsyncStackSnapshot::FromEtsObject(EtsObject::FromCoreType(object));
}

PandaEtsVM *EtsAsyncStackSnapshotHandle::GetOwningVM() const
{
    return vm_;
}

}  // namespace ark::ets
