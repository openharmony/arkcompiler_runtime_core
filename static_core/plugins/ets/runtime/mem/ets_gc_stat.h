/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef ETS_GC_STAT_H
#define ETS_GC_STAT_H

#include "mem/gc/gc.h"

namespace ark::ets {

class FullGCLongTimeListener : public ark::mem::GCListener {
public:
    static constexpr uint64_t LONG_GC_THRESHOLD_NS = 40'000'000;
    FullGCLongTimeListener() = default;
    ~FullGCLongTimeListener() override = default;
    PANDA_PUBLIC_API static FullGCLongTimeListener *CreateGCListener();
    void GCStarted(const ark::GCTask &task, size_t heapSize) override;
    void GCFinished(const ark::GCTask &task, size_t heapSizeBeforeGc, size_t heapSize) override;
    uint64_t GetFullGCLongTimeCount() const;

private:
    uint64_t startTime_ = 0;
    uint64_t fullGCLongTimeCounter_ = 0;
    NO_COPY_SEMANTIC(FullGCLongTimeListener);
    NO_MOVE_SEMANTIC(FullGCLongTimeListener);
};

}  // namespace ark::ets
#endif  // ETS_GC_STAT_H
