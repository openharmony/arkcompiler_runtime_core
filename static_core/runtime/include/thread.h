/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_THREAD_H_
#define PANDA_RUNTIME_THREAD_H_

#include <cstdint>
#include <csignal>

#include "libarkbase/mem/gc_barrier.h"
#include "libarkbase/mem/ringbuf/lock_free_ring_buffer.h"
#include "libarkbase/mem/weighted_adaptive_tlab_average.h"
#include "libarkbase/os/mutex.h"
#include "libarkbase/os/thread.h"
#include "libarkbase/utils/arch.h"
#include "libarkbase/utils/list.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/tsan_interface.h"
#include "runtime/include/object_header-inl.h"
#include "runtime/include/stack_walker.h"
#include "runtime/include/language_context.h"
#include "runtime/interpreter/cache.h"
#include "runtime/mem/frame_allocator-inl.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/internal_allocator.h"
#include "runtime/mem/refstorage/reference_storage.h"
#include "runtime/entrypoints/entrypoints.h"
#include "libarkbase/events/events.h"
#ifdef PANDA_USE_QOS
#include "qos.h"
#endif

enum Priority {
    PRIORITY_IDLE = 0,
    PRIORITY_LOW = 1,
    PRIORITY_DEFAULT = 2,
    PRIORITY_HIGH = 3,
    PRIORITY_DEADLINE_REQUEST = 4,
    PRIORITY_USER_INTERACTION = 5,
};

#define ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS()

namespace ark {

class ThreadManager;
class Runtime;
class PandaVM;
class MonitorPool;

/// @brief Class represents arbitrary runtime thread
class Thread {
public:
    using ThreadId = uint32_t;

    explicit Thread(PandaVM *vm);
    virtual ~Thread();
    NO_COPY_SEMANTIC(Thread);
    NO_MOVE_SEMANTIC(Thread);

    PandaVM *GetPandaVM() const
    {
        return vm_;
    }

    void SetVM(PandaVM *vm)
    {
        vm_ = vm;
    }

private:
    PandaVM *vm_ {nullptr};
#if defined(PANDA_USE_CUSTOM_SIGNAL_STACK)
    FIELD_UNUSED stack_t signalStack_ {};
#endif  // PANDA_USE_CUSTOM_SIGNAL_STACK
};

class QosHelper {
public:
    static int SetCurrentWorkerPriority([[maybe_unused]] Priority priority)
    {
#ifdef PANDA_USE_QOS
        return SetThreadQos(static_cast<OHOS::QOS::QosLevel>(priority));
#else
        LOG(INFO, RUNTIME) << "QosHelper::SetWorkerPriority is not supported";
        return 0;
#endif
    }
};

}  // namespace ark

#endif  // PANDA_RUNTIME_THREAD_H_
