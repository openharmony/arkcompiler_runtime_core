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
#ifndef PANDA_RUNTIME_EXECUTION_ASYNC_STACK_SNAPSHOT_HANDLE_H
#define PANDA_RUNTIME_EXECUTION_ASYNC_STACK_SNAPSHOT_HANDLE_H

#include <cstdint>
#include <memory>

#include <libarkbase/macros.h>
#include <libarkbase/os/mutex.h>

#include "runtime/execution/async_stack_snapshot_view.h"

namespace ark {

/**
 * @brief Language-independent owner of an async stack snapshot.
 *
 * Runtime common code does not know the managed representation of a snapshot. A language plugin
 * creates a handle which owns a VM-scoped strong root and provides this interface.
 */
class AsyncStackSnapshotHandle {
public:
    using Ptr = std::unique_ptr<AsyncStackSnapshotHandle>;

    AsyncStackSnapshotHandle() = default;
    virtual ~AsyncStackSnapshotHandle() = default;

    NO_COPY_SEMANTIC(AsyncStackSnapshotHandle);
    NO_MOVE_SEMANTIC(AsyncStackSnapshotHandle);

    /**
     * @brief Creates another owner of the same snapshot.
     *
     * The returned handle must own a separate strong root and remain valid after this handle is
     * destroyed.
     */
    virtual Ptr Clone() const = 0;

    /**
     * @brief Returns an opaque type tag identifying the plugin-specific snapshot representation.
     *
     * Runtime common code never interprets the value. A plugin adapter compares it with its own
     * tag before accessing GetOpaqueSnapshot().
     */
    virtual const void *GetSnapshotTypeId() const noexcept
    {
        return nullptr;
    }

    /**
     * @brief Returns an opaque pointer to the plugin-owned managed snapshot.
     *
     * Only code that recognizes GetSnapshotTypeId() may cast this pointer.
     */
    virtual void *GetOpaqueSnapshot() const noexcept
    {
        return nullptr;
    }

    /**
     * @brief Returns an opaque owner of the snapshot, for example the owning VM.
     *
     * A plugin adapter may compare it with a known owner before interpreting the snapshot.
     */
    virtual void *GetOpaqueOwner() const noexcept
    {
        return nullptr;
    }

    /**
     * @brief Creates a native copy of the snapshot on the current mutator thread.
     *
     * The default implementation is used by language plugins that do not expose debugger snapshots.
     * It must be called only while the thread owning the snapshot is paused or otherwise synchronized.
     */
    virtual std::unique_ptr<AsyncStackSnapshotView> CreateSnapshotView() const
    {
        return nullptr;
    }
};

using AsyncStackSnapshotHandlePtr = AsyncStackSnapshotHandle::Ptr;

/** Immutable view of the VM-local async debugger configuration. */
struct AsyncDebuggerConfigSnapshot {
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    uint32_t maxAsyncDepth = 0U;
    uint32_t maxFramesPerSegment = 0U;
    uint64_t generation = 0U;
    // NOLINTEND(misc-non-private-member-variables-in-classes)

    bool IsCaptureEnabled() const
    {
        return maxAsyncDepth != 0U;
    }
};

/**
 * @brief VM-local, thread-safe configuration of debugger async stack capture.
 *
 * A positive depth enables capture. A transition from disabled to enabled increments generation,
 * so snapshots created before a disable/enable cycle cannot be inherited as a parent chain.
 */
class AsyncDebuggerConfig {
public:
    static constexpr uint32_t DEFAULT_MAX_ASYNC_DEPTH = 8U;
    static constexpr uint32_t DEFAULT_MAX_FRAMES_PER_SEGMENT = 48U;

    AsyncDebuggerConfigSnapshot GetSnapshot() const
    {
        os::memory::LockHolder lock(mutex_);
        return AsyncDebuggerConfigSnapshot {enabled_ ? maxAsyncDepth_ : 0U, maxFramesPerSegment_, generation_};
    }

    void SetEnabled(bool enabled)
    {
        os::memory::LockHolder lock(mutex_);
        SetEnabledLocked(enabled);
    }

    void SetMaxAsyncDepth(uint32_t maxAsyncDepth)
    {
        os::memory::LockHolder lock(mutex_);
        if (maxAsyncDepth == 0U) {
            SetEnabledLocked(false);
            return;
        }

        maxAsyncDepth_ = maxAsyncDepth;
        SetEnabledLocked(true);
    }

    bool SetMaxFramesPerSegment(uint32_t maxFramesPerSegment)
    {
        if (maxFramesPerSegment == 0U) {
            return false;
        }

        os::memory::LockHolder lock(mutex_);
        maxFramesPerSegment_ = maxFramesPerSegment;
        return true;
    }

private:
    void SetEnabledLocked(bool enabled)
    {
        if (enabled == enabled_) {
            return;
        }

        enabled_ = enabled;
        generation_ += enabled ? 1U : 0U;
    }

    mutable os::memory::Mutex mutex_;
    bool enabled_ = false;
    uint32_t maxAsyncDepth_ = DEFAULT_MAX_ASYNC_DEPTH;
    uint32_t maxFramesPerSegment_ = DEFAULT_MAX_FRAMES_PER_SEGMENT;
    uint64_t generation_ = 0U;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_EXECUTION_ASYNC_STACK_SNAPSHOT_HANDLE_H
