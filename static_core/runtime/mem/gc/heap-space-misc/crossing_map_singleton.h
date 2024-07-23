/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#ifndef PANDA_MEM_GC_CROSSING_MAP_SINGLETON_H
#define PANDA_MEM_GC_CROSSING_MAP_SINGLETON_H

#include <cstdlib>

#include "libpandabase/macros.h"
#include "libpandabase/mem/mem_range.h"
#include "libpandabase/os/mutex.h"

namespace ark::mem {

class CrossingMap;
/// Singleton for CrossingMap class.
class CrossingMapSingleton {
public:
    CrossingMapSingleton() = delete;
    ~CrossingMapSingleton() = delete;
    NO_COPY_SEMANTIC(CrossingMapSingleton);
    NO_MOVE_SEMANTIC(CrossingMapSingleton);

    static bool Create();
    static bool IsCreated();

    static CrossingMap *GetCrossingMap()
    {
        ASSERT(instance_ != nullptr);
        return instance_;
    }

    static void AddObject(void *objAddr, size_t objSize);
    static void RemoveObject(void *objAddr, size_t objSize, void *nextObjAddr, void *prevObjAddr, size_t prevObjSize);
    static void *FindFirstObject(void *startAddr, void *endAddr);

    static void InitializeCrossingMapForMemory(void *startAddr, size_t size);
    static void RemoveCrossingMapForMemory(void *startAddr, size_t size);

    static bool Destroy();

    static size_t GetCrossingMapGranularity();

    // NOTE(dtrubenkov): move it to the more proper place
    static void MarkCardsAsYoung(const MemRange &memRange);

private:
    static CrossingMap *instance_;
    static os::memory::Mutex mutex_;
};

}  // namespace ark::mem
#endif  // PANDA_MEM_GC_CROSSING_MAP_SINGLETON_H
