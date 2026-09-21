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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_DFX_ETS_ASYNC_STACK_SNAPSHOT_HANDLE_H
#define PANDA_PLUGINS_ETS_RUNTIME_DFX_ETS_ASYNC_STACK_SNAPSHOT_HANDLE_H

#include "runtime/execution/async_stack_snapshot_handle.h"

namespace ark::mem {
class Reference;
}  // namespace ark::mem

namespace ark::ets {

namespace test {
class EtsAsyncStackSnapshotHandleFailureTest;
}  // namespace test

class EtsAsyncStackSnapshot;
class PandaEtsVM;

/**
 * Owns one VM-scoped global root for an immutable managed async stack snapshot.
 *
 * The root is not tied to the worker that created it, so the handle can move between jobs in the
 * same VM. Removing the root in the destructor is the only release path.
 */
class EtsAsyncStackSnapshotHandle final : public AsyncStackSnapshotHandle {
public:
    NO_COPY_SEMANTIC(EtsAsyncStackSnapshotHandle);
    NO_MOVE_SEMANTIC(EtsAsyncStackSnapshotHandle);

    static AsyncStackSnapshotHandlePtr Create(PandaEtsVM *vm, EtsAsyncStackSnapshot *snapshot);
    static const void *SnapshotTypeId();

    ~EtsAsyncStackSnapshotHandle() override;

    AsyncStackSnapshotHandlePtr Clone() const override;
    std::unique_ptr<AsyncStackSnapshotView> CreateSnapshotView() const override;
    const void *GetSnapshotTypeId() const noexcept override;
    void *GetOpaqueSnapshot() const noexcept override;
    void *GetOpaqueOwner() const noexcept override;

    EtsAsyncStackSnapshot *GetSnapshot() const noexcept;
    PandaEtsVM *GetOwningVM() const;

private:
    /** Test-only factory which simulates native allocation failure after root insertion. */
    static AsyncStackSnapshotHandlePtr CreateWithAllocationFailureForTest(PandaEtsVM *vm,
                                                                          EtsAsyncStackSnapshot *snapshot);

    explicit EtsAsyncStackSnapshotHandle(PandaEtsVM *vm, mem::Reference *reference);

    static AsyncStackSnapshotHandlePtr Create(PandaEtsVM *vm, EtsAsyncStackSnapshot *snapshot,
                                              bool failNativeAllocation);

    friend class ::ark::ets::test::EtsAsyncStackSnapshotHandleFailureTest;

    PandaEtsVM *vm_;
    mem::Reference *reference_;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_DFX_ETS_ASYNC_STACK_SNAPSHOT_HANDLE_H
