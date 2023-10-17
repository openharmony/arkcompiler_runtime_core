/**
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

#include "libpandabase/os/cpu_affinity.h"
#include "libpandabase/utils/logger.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <thread>

namespace panda::os {

CpuSet::CpuSet()  // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    CPU_ZERO(&cpuset_);
}

void CpuSet::Set(int cpu)
{
    CPU_SET(cpu, &cpuset_);
}

void CpuSet::Clear()
{
    CPU_ZERO(&cpuset_);
}

void CpuSet::Remove(int cpu)
{
    CPU_CLR(cpu, &cpuset_);
}

size_t CpuSet::Count() const
{
    return CPU_COUNT(&cpuset_);
}

bool CpuSet::IsSet(int cpu) const
{
    return CPU_ISSET(cpu, &cpuset_);
}

bool CpuSet::IsEmpty() const
{
    return Count() == 0U;
}

CpuSetType *CpuSet::GetData()
{
    return &cpuset_;
}

const CpuSetType *CpuSet::GetData() const
{
    return &cpuset_;
}

bool CpuSet::operator==(const CpuSet &other) const
{
    return CPU_EQUAL(&cpuset_, other.GetData());
}

bool CpuSet::operator!=(const CpuSet &other) const
{
    return !(*this == other);
}

/* static */
void CpuSet::And(CpuSet &result, const CpuSet &lhs, const CpuSet &rhs)
{
    CPU_AND(result.GetData(), lhs.GetData(), rhs.GetData());
}

/* static */
void CpuSet::Or(CpuSet &result, const CpuSet &lhs, const CpuSet &rhs)
{
    CPU_OR(result.GetData(), lhs.GetData(), rhs.GetData());
}

/* static */
void CpuSet::Xor(CpuSet &result, const CpuSet &lhs, const CpuSet &rhs)
{
    CPU_XOR(result.GetData(), lhs.GetData(), rhs.GetData());
}

/* static */
void CpuSet::Copy(CpuSet &destination, const CpuSet &source)
{
    memcpy(destination.GetData(), source.GetData(), sizeof(CpuSetType));
    ASSERT(destination == source);
}

CpuSet &CpuSet::operator&=(const CpuSet &other)
{
    CPU_AND(&cpuset_, &cpuset_, other.GetData());
    return *this;
}

CpuSet &CpuSet::operator|=(const CpuSet &other)
{
    CPU_OR(&cpuset_, &cpuset_, other.GetData());
    return *this;
}

CpuSet &CpuSet::operator^=(const CpuSet &other)
{
    CPU_XOR(&cpuset_, &cpuset_, other.GetData());
    return *this;
}

size_t CpuAffinityManager::cpu_count_ = 0U;
CpuSet CpuAffinityManager::best_cpu_set_;             // NOLINT(fuchsia-statically-constructed-objects)
CpuSet CpuAffinityManager::middle_cpu_set_;           // NOLINT(fuchsia-statically-constructed-objects)
CpuSet CpuAffinityManager::best_and_middle_cpu_set_;  // NOLINT(fuchsia-statically-constructed-objects)
CpuSet CpuAffinityManager::weak_cpu_set_;             // NOLINT(fuchsia-statically-constructed-objects)

/* static */
void CpuAffinityManager::Initialize()
{
    ASSERT(!IsCpuAffinityEnabled());
    ASSERT(best_cpu_set_.IsEmpty());
    ASSERT(middle_cpu_set_.IsEmpty());
    ASSERT(best_and_middle_cpu_set_.IsEmpty());
    ASSERT(weak_cpu_set_.IsEmpty());
    cpu_count_ = std::thread::hardware_concurrency();
    LoadCpuFreq();
}

/* static */
bool CpuAffinityManager::IsCpuAffinityEnabled()
{
    return cpu_count_ != 0U;
}

/* static */
void CpuAffinityManager::Finalize()
{
    cpu_count_ = 0U;
    best_cpu_set_.Clear();
    middle_cpu_set_.Clear();
    best_and_middle_cpu_set_.Clear();
    weak_cpu_set_.Clear();
}

/* static */
void CpuAffinityManager::LoadCpuFreq()
{
    if (!IsCpuAffinityEnabled()) {
        return;
    }
    std::vector<CpuInfo> cpu;
    cpu.reserve(cpu_count_);
    for (size_t cpu_num = 0; cpu_num < cpu_count_; ++cpu_num) {
        std::string cpu_freq_path =
            "/sys/devices/system/cpu/cpu" + std::to_string(cpu_num) + "/cpufreq/cpuinfo_max_freq";
        std::ifstream cpu_freq_file(cpu_freq_path.c_str());
        uint64_t freq = 0U;
        if (cpu_freq_file.is_open()) {
            if (!(cpu_freq_file >> freq)) {
                freq = 0U;
            } else {
                cpu.push_back({cpu_num, freq});
            }
            cpu_freq_file.close();
        }
        if (freq == 0U) {
            cpu_count_ = 0;
            return;
        }
    }
    // Sort by cpu frequency from best to weakest
    std::sort(cpu.begin(), cpu.end(), [](const CpuInfo &lhs, const CpuInfo &rhs) { return lhs.freq > rhs.freq; });
    auto cpu_info = cpu.front();
    best_cpu_set_.Set(cpu_info.number);
    for (auto it = cpu.begin() + 1U; it != cpu.end(); ++it) {
        if (it->freq == cpu_info.freq) {
            best_cpu_set_.Set(it->number);
        } else {
            break;
        }
    }
    cpu_info = cpu.back();
    weak_cpu_set_.Set(cpu_info.number);
    for (auto it = cpu.rbegin() + 1U; it != cpu.rend(); ++it) {
        if (it->freq == cpu_info.freq) {
            weak_cpu_set_.Set(it->number);
        } else {
            break;
        }
    }
    if (best_cpu_set_.Count() + weak_cpu_set_.Count() >= cpu_count_) {
        CpuSet::Copy(middle_cpu_set_, weak_cpu_set_);
    } else {
        for (auto it = cpu.begin() + best_cpu_set_.Count(); it != cpu.end() - weak_cpu_set_.Count(); ++it) {
            middle_cpu_set_.Set(it->number);
        }
        ASSERT(best_cpu_set_.Count() + middle_cpu_set_.Count() + weak_cpu_set_.Count() == cpu_count_);
    }
    ASSERT(!best_cpu_set_.IsEmpty());
    ASSERT(!middle_cpu_set_.IsEmpty());
    ASSERT(!weak_cpu_set_.IsEmpty());
    CpuSet::Or(best_and_middle_cpu_set_, best_cpu_set_, middle_cpu_set_);
    ASSERT(!best_and_middle_cpu_set_.IsEmpty());
    for (auto it : cpu) {
        LOG(DEBUG, COMMON) << "CPU number: " << it.number << ", freq: " << it.freq;
    }
}

/* static */
bool CpuAffinityManager::GetThreadAffinity(int tid, CpuSet &cpuset)
{
    bool success = sched_getaffinity(tid, sizeof(CpuSetType), cpuset.GetData()) == 0;
    LOG_IF(!success, DEBUG, COMMON) << "Couldn't get affinity for thread with tid = "
                                    << (tid != 0 ? tid : thread::GetCurrentThreadId())
                                    << ", error: " << std::strerror(errno);
    return success;
}

/* static */
bool CpuAffinityManager::GetCurrentThreadAffinity(CpuSet &cpuset)
{
    // 0 is value for current thread
    return GetThreadAffinity(0, cpuset);
}

/* static */
bool CpuAffinityManager::SetAffinityForThread(int tid, const CpuSet &cpuset)
{
    if (!IsCpuAffinityEnabled()) {
        return false;
    }
    bool success = sched_setaffinity(tid, sizeof(CpuSetType), cpuset.GetData()) == 0;
    LOG_IF(!success, DEBUG, COMMON) << "Couldn't set affinity with mask " << CpuSetToString(cpuset)
                                    << " for thread with tid = " << (tid != 0 ? tid : thread::GetCurrentThreadId())
                                    << ", error: " << std::strerror(errno);
    return success;
}

/* static */
bool CpuAffinityManager::SetAffinityForThread(int tid, uint8_t power_flags)
{
    if (power_flags == CpuPower::ANY) {
        return true;
    }
    CpuSet cpuset;
    if ((power_flags & CpuPower::BEST) != 0) {
        cpuset |= best_cpu_set_;
    }
    if ((power_flags & CpuPower::MIDDLE) != 0) {
        cpuset |= middle_cpu_set_;
    }
    if ((power_flags & CpuPower::WEAK) != 0) {
        cpuset |= weak_cpu_set_;
    }
    return SetAffinityForThread(tid, cpuset);
}

/* static */
bool CpuAffinityManager::SetAffinityForCurrentThread(uint8_t power_flags)
{
    // 0 is value for current thread
    return SetAffinityForThread(0, power_flags);
}

/* static */
bool CpuAffinityManager::SetAffinityForCurrentThread(const CpuSet &cpuset)
{
    // 0 is value for current thread
    return SetAffinityForThread(0, cpuset);
}

}  // namespace panda::os
