/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef ARK_RUNTIME_GC_HUNG_H
#define ARK_RUNTIME_GC_HUNG_H

#include <sys/syscall.h>
#include <unistd.h>
#include <string>
#include "libpandabase/utils/time.h"
#include "libpandabase/os/library_loader.h"
#include "runtime/include/gc_task.h"
#include "runtime/include/mtmanaged_thread.h"
#include "runtime/include/gc_task.h"

namespace panda::mem {

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
const char LIB_IMONITOR[] = "libimonitor.so";
const unsigned int ZRHUNG_WP_GC = 8;
const uint64_t INTERVAL_LIMIT_MS_INIT = 50;
const uint64_t OVER_TIME_LIMIT_INIT_MS = 2000;
const uint64_t WATER_MARK_LIMIT = 20;

// zrhung functions type define
using ZRHUNG_SEND_EVENT = int (*)(int16_t, const char *, const char *);
using ZRHUNG_GET_CONFIG = int (*)(int16_t, char *, uint32_t);

enum GcPara {
    GC_PARA_ENABLE,
    GC_PARA_INTERVAL,  // ms
    GC_PARA_WATERMARK,
    GC_PARA_OVERTIME,  // ms
    GC_PARA_COUNT
};

// Functions of the Hung Check Function in the Ark VM,record gc's anomalous behavior
class GcHung {
public:
    GcHung();
    ~GcHung();
    GcHung(const GcHung &) = delete;
    GcHung &operator=(GcHung const &) = delete;
    GcHung(GcHung &&) = delete;
    GcHung &operator=(GcHung &&) = delete;
    void SendZerohungEvent(const PandaString &error, int pid, PandaString msg);
    bool IsConfigReady() const
    {
        return config_ready_;
    }
    bool IsEnabled() const
    {
        return enabled_;
    }
    void SetEnabled(bool enabled)
    {
        enabled_ = enabled;
    }
    bool IsReady() const
    {
        return ready_;
    }
    static void InitPreFork(bool enabled);
    static void InitPostFork(bool is_systemserver);
    static GcHung *Current();
    static void Start();
    static void Check(const GCTask &task);
    // NOLINTNEXTLINE(google-runtime-references)
    static void Check(const PandaList<MTManagedThread *> &threads, uint64_t start_time);
    static bool UpdateConfig();

private:
    static GcHung *instance_;
    pid_t pid_ {-1};
    bool enabled_ {false};
    bool ready_ {false};
    uint64_t interval_limit_ms_ {0};
    uint64_t over_time_limit_ms_ {0};
    uint64_t last_gc_time_ns_ {0};
    uint64_t congestion_duration_ns_ {0};
    int water_mark_limit_ {0};
    int water_mark_ {0};
    int report_count_ {0};
    bool config_ready_ {false};
    bool is_systemserver_ {false};
    uint64_t start_time_ns_ {0};
    os::library_loader::LibraryHandle libimonitor_dl_handler_ {nullptr};
    ZRHUNG_SEND_EVENT zrhung_send_event_ {nullptr};
    ZRHUNG_GET_CONFIG zrhung_get_config_ {nullptr};

    int GetConfig();
    int LoadLibimonitor();
    void ReportGcCongestion();
    void ReportSuspendTimedout();
    void InitInternal(bool is_systemserver);
    void CheckSuspend(const PandaList<MTManagedThread *> &threads, uint64_t start_time);
    void CheckFrequency();
    void CheckOvertime(const GCTask &task);
    void UpdateStartTime();
};

class ScopedGcHung {
public:
    explicit ScopedGcHung(const GCTask *task_start)
    {
        GcHung::Start();
        task = task_start;
    }

    ~ScopedGcHung()
    {
        GcHung::Check(*task);
    }

private:
    const GCTask *task;

    NO_COPY_SEMANTIC(ScopedGcHung);
    NO_MOVE_SEMANTIC(ScopedGcHung);
};

}  // namespace panda::mem
#endif  // ARK_RUNTIME_GC_HUNG_H