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

#include "gc_hung.h"

#include <unistd.h>
#include <cstdlib>
#include <dlfcn.h>
#include <ostream>
#include <sstream>
#include <sys/types.h>
#include <ctime>
#include <cinttypes>
#include <cstdio>
#include <csignal>
#include <sys/time.h>
#include "libpandabase/utils/logger.h"
#include "libpandabase/utils/type_converter.h"

namespace panda::mem {

/*
 * If the macro HUNG_SYSTEM_SERVER_ONLY was defined, the module only
 * supervise system_server. Otherwise, all the processes running in ARK
 */
using std::string;

GcHung *GcHung::instance_ = nullptr;

// NOLINT(google-runtime-references)
static void Split(const PandaString &str, char delim, PandaVector<PandaString> *elems, bool skipEmpty = true)
{
    PandaIStringStream iss(str);
    for (PandaString item; getline(iss, item, delim);) {
        if (!(skipEmpty && item.empty())) {
            elems->push_back(item);
        }
    }
}

GcHung::GcHung()
    : interval_limit_ms_(INTERVAL_LIMIT_MS_INIT),
      over_time_limit_ms_(OVER_TIME_LIMIT_INIT_MS),
      water_mark_limit_(WATER_MARK_LIMIT)
{
    LOG(DEBUG, GC) << "GcHung: Instance created";
}

GcHung::~GcHung()
{
    if (libimonitor_dl_handler_.GetNativeHandle() != nullptr) {
        dlclose(libimonitor_dl_handler_.GetNativeHandle());
    }
    LOG(DEBUG, GC) << "GcHung: Instance deleted: water_mark: " << water_mark_;
}

int GcHung::GetConfig()
{
    const unsigned int maxParaLen = 100;
    // get parameter from zrhung
    if (zrhung_get_config_ == nullptr) {
        return -1;
    }

    char paraBuf[maxParaLen] = {0};  // NOLINT(modernize-avoid-c-arrays)
    if (zrhung_get_config_(ZRHUNG_WP_GC, paraBuf, maxParaLen) != 0) {
        LOG(DEBUG, GC) << "GcHung: failed to get config";
        return -1;
    }

    PandaString paraStr(paraBuf);
    PandaVector<PandaString> paraVec;
    Split(paraStr, ',', &paraVec);

    if (paraVec.size() != GC_PARA_COUNT) {
        LOG(ERROR, GC) << "GcHung: parse parameters failed";
        return -1;
    }

#ifdef HUNG_SYSTEM_SERVER_ONLY
    enabled_ = (stoi(PandaStringToStd(paraVec[GC_PARA_ENABLE])) == 1) && is_systemserver_;
#else
    enabled_ = (stoi(PandaStringToStd(paraVec[GC_PARA_ENABLE])) == 1);
#endif  // HUNG_SYSTEM_SERVER_ONLY
    LOG(INFO, GC) << "GcHung: module enable:" << enabled_;

    interval_limit_ms_ = static_cast<uint64_t>(std::stoi(PandaStringToStd(paraVec[GC_PARA_INTERVAL])));
    water_mark_limit_ = std::stoi(PandaStringToStd(paraVec[GC_PARA_WATERMARK]));
    over_time_limit_ms_ = static_cast<uint64_t>(std::stoi(PandaStringToStd(paraVec[GC_PARA_OVERTIME])));
    LOG(DEBUG, GC) << "GcHung: set parameter: interval_limit_ms_ = " << interval_limit_ms_ << "ms";
    LOG(DEBUG, GC) << "GcHung: set parameter: water_mark_limit_ = " << water_mark_limit_;
    LOG(DEBUG, GC) << "GcHung: set parameter: over_time_limit_ms_ = " << over_time_limit_ms_ << "ms";
    config_ready_ = true;

    return 0;
}

int GcHung::LoadLibimonitor()
{
    if ((zrhung_send_event_ != nullptr) && (zrhung_get_config_ != nullptr)) {
        return 1;
    }
    LOG(DEBUG, GC) << "GcHung: load libimonitor";
    auto res_load = os::library_loader::Load(LIB_IMONITOR);
    if (!res_load) {
        LOG(ERROR, RUNTIME) << "failed to load " << LIB_IMONITOR << " Error: " << res_load.Error().ToString();
        return -1;
    }
    libimonitor_dl_handler_ = std::move(res_load.Value());

    auto zrhung_send_event_dlsym = os::library_loader::ResolveSymbol(libimonitor_dl_handler_, "zrhung_send_event");
    if (!zrhung_send_event_dlsym) {
        LOG(ERROR, RUNTIME) << "failed to dlsym symbol: zrhung_send_event";
        dlclose(libimonitor_dl_handler_.GetNativeHandle());
        return -1;
    }
    zrhung_send_event_ = reinterpret_cast<ZRHUNG_SEND_EVENT>(zrhung_send_event_dlsym.Value());

    auto zrhung_get_config_dlsym = os::library_loader::ResolveSymbol(libimonitor_dl_handler_, "zrhung_get_config");
    if (!zrhung_get_config_dlsym) {
        LOG(ERROR, RUNTIME) << "failed to dlsym symbol: zrhung_get_config";
        dlclose(libimonitor_dl_handler_.GetNativeHandle());
        return -1;
    }
    zrhung_get_config_ = reinterpret_cast<ZRHUNG_GET_CONFIG>(zrhung_get_config_dlsym.Value());
    return 0;
}

void GcHung::InitInternal(bool is_systemserver)
{
    pid_ = getpid();
    last_gc_time_ns_ = 0;
    congestion_duration_ns_ = 0;
    water_mark_ = 0;
    report_count_ = 0;
    start_time_ns_ = 0;
    is_systemserver_ = is_systemserver;
#ifdef HUNG_SYSTEM_SERVER_ONLY
    ready_ = is_systemserver;  // if is_systemserver == false, hung will be close and no way to open again
#else
    ready_ = true;
#endif  // HUNG_SYSTEM_SERVER_ONLY

    LOG(DEBUG, GC) << "GcHung InitInternal: pid=" << pid_ << " enabled_=" << enabled_;
}

void GcHung::SendZerohungEvent(const PandaString &error, int pid, PandaString msg)
{
    msg = ">>>*******************" + error + "******************\n" + msg;
    if ((zrhung_send_event_ == nullptr) || (zrhung_get_config_ == nullptr)) {
        LOG(ERROR, GC) << "GcHung: zrhung functions not defined";
        return;
    }
    if (pid > 0) {
        PandaString command = "P=" + ToPandaString(pid);
        zrhung_send_event_(ZRHUNG_WP_GC, command.c_str(), msg.c_str());
    } else {
        zrhung_send_event_(ZRHUNG_WP_GC, nullptr, msg.c_str());
    }
}

// check threads suspend while get "Locks::mutator_lock->WriteLock()", and report to hung
void GcHung::CheckSuspend(const PandaList<MTManagedThread *> &threads, uint64_t start_time)
{
    LOG(DEBUG, GC) << "GcHung: check suspend timeout";
    PandaOStringStream oss;
    for (const auto &thread : threads) {
        if (thread == nullptr) {
            continue;
        }
        if (!thread->IsSuspended()) {
            auto tid = thread->GetId();

            oss << "GcHung: Timed out waiting for thread " << tid << " to suspend, waited for "
                << helpers::TimeConverter(time::GetCurrentTimeInNanos() - start_time) << std::endl;
        }
    }
    LOG(ERROR, GC) << oss.str();

    if (config_ready_ && enabled_) {
        SendZerohungEvent("SuspendAll timed out", getpid(), oss.str());
    }
}

void GcHung::CheckFrequency()
{
    LOG(DEBUG, GC) << "GcHung: gc frequency check: PID = " << pid_
                   << " last_gc_time_ns_=" << helpers::TimeConverter(last_gc_time_ns_)
                   << " current_time=" << helpers::TimeConverter(time::GetCurrentTimeInNanos());

    if (last_gc_time_ns_ == 0) {
        last_gc_time_ns_ = time::GetCurrentTimeInNanos();
        return;
    }

    using RESULT_DURATION = std::chrono::duration<uint64_t, std::deca>;
    std::chrono::microseconds msec(interval_limit_ms_);
    if ((start_time_ns_ - last_gc_time_ns_) < std::chrono::duration_cast<RESULT_DURATION>(msec).count()) {
        water_mark_++;
        congestion_duration_ns_ += (time::GetCurrentTimeInNanos() - last_gc_time_ns_);
        LOG(DEBUG, GC) << "GcHung: proc " << pid_ << " water_mark_:" << water_mark_
                       << " duration:" << helpers::TimeConverter(time::GetCurrentTimeInNanos() - last_gc_time_ns_);
    } else {
        water_mark_ = 0;
        congestion_duration_ns_ = 0;
    }

    if (water_mark_ > water_mark_limit_) {
        PandaOStringStream oss;
        oss << "GcHung: GC congestion PID:" << pid_ << " Freq:" << water_mark_ << "/"
            << helpers::TimeConverter(congestion_duration_ns_);

        LOG(ERROR, GC) << oss.str();
        if (config_ready_ && enabled_) {
            SendZerohungEvent("GC congestion", -1, oss.str());  // -1: invalid pid
        }
        water_mark_ = 0;
        congestion_duration_ns_ = 0;
    }
    last_gc_time_ns_ = time::GetCurrentTimeInNanos();
}

void GcHung::CheckOvertime(const GCTask &task)
{
    uint64_t gcTime = time::GetCurrentTimeInNanos() - start_time_ns_;
    LOG(DEBUG, GC) << "GcHung: gc overtime check: start_time_ns_=" << helpers::TimeConverter(start_time_ns_)
                   << " current_time=" << helpers::TimeConverter(time::GetCurrentTimeInNanos())
                   << " total_time=" << helpers::TimeConverter(gcTime);
    using RESULT_DURATION = std::chrono::duration<uint64_t, std::deca>;
    std::chrono::microseconds msec(over_time_limit_ms_);
    if (gcTime > std::chrono::duration_cast<RESULT_DURATION>(msec).count()) {
        PandaOStringStream oss;
        oss << "GcHung: GC overtime: total:" << helpers::TimeConverter(gcTime) << " cause: " << task.reason_;
        LOG(ERROR, GC) << oss.str();
        if (config_ready_ && enabled_) {
            SendZerohungEvent("GC overtime", -1, oss.str());  // -1: invalid pid
        }
    }
}

void GcHung::UpdateStartTime()
{
    start_time_ns_ = time::GetCurrentTimeInNanos();
}

void GcHung::Start()
{
    if (instance_ == nullptr) {
        return;
    }
    if (!instance_->IsEnabled() || !instance_->IsReady()) {
        return;
    }
    instance_->UpdateStartTime();
}

void GcHung::Check(const GCTask &task)
{
    if (instance_ == nullptr) {
        LOG(WARNING, GC) << "GcHung not initiated yet";
        return;
    }
    if (!instance_->IsEnabled() || !instance_->IsReady()) {
        return;
    }
    if (task.reason_ != GCTaskCause::NATIVE_ALLOC_CAUSE) {
        instance_->CheckFrequency();
    }
    instance_->CheckOvertime(task);
}

// NOLINTNEXTLINE(google-runtime-references)
void GcHung::Check(const PandaList<MTManagedThread *> &threads, uint64_t start_time)
{
    if (instance_ != nullptr) {
        instance_->CheckSuspend(threads, start_time);
    } else {
        LOG(INFO, GC) << "GcHung not initiated yet, skip checking";
    }
}

bool GcHung::UpdateConfig()
{
    if (instance_ == nullptr) {
        LOG(ERROR, GC) << "GcHung Update Config failed, GcHung not initiated yet";
        return false;
    }
    if (!instance_->IsReady()) {
        LOG(ERROR, GC) << "GcHung Update Config failed, hung not ready";
        return false;
    }
    if (instance_->GetConfig() != 0) {
        LOG(ERROR, GC) << "GcHung Update Config failed, GetConfig again failed";
        return false;
    }
    LOG(ERROR, GC) << "GcHung Update Config success";
    return true;
}

void GcHung::InitPreFork(bool enabled)
{
    LOG(DEBUG, GC) << "GcHung: InitPreFork";
    if (instance_ == nullptr) {
        instance_ = new GcHung();
    }
    instance_->SetEnabled(enabled);
    if (enabled) {
        instance_->LoadLibimonitor();
        instance_->GetConfig();
    }
}

void GcHung::InitPostFork(bool is_systemserver)
{
    LOG(DEBUG, GC) << "GcHung: InitPostFork";
    if (instance_ != nullptr) {
        instance_->InitInternal(is_systemserver);
    }
}

GcHung *GcHung::Current()
{
    return instance_;
}

}  // namespace panda::mem