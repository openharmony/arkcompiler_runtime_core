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
#ifndef PANDA_RUNTIME_INCLUDE_TOOLING_DEBUG_INF_H
#define PANDA_RUNTIME_INCLUDE_TOOLING_DEBUG_INF_H

#include <atomic>
#include "include/thread.h"
#include "libpandafile/file.h"
#include "libpandabase/os/mutex.h"
#include "libpandabase/utils/time.h"
#include "libpandabase/utils/span.h"

namespace ark::tooling {
struct PCodeItem;
struct PCodeMetaInfo;

class DebugInf {
public:
    DebugInf() = default;
    ~DebugInf() = default;

    // interface for runtime.
    // when loading new panda file, add new entry to list
    static void AddCodeMetaInfo(const panda_file::File *file);
    // when unloading new panda file, remove entry to list
    static void DelCodeMetaInfo(const panda_file::File *file);

private:
    static void Lock(PCodeMetaInfo *mi);
    static void UnLock(PCodeMetaInfo *mi);
    static PCodeItem *AddCodeMetaInfoImpl(PCodeMetaInfo *metaInfo, Span<const uint8_t> inss);
    static void DelCodeMetaInfoImpl(PCodeMetaInfo *metaInfo, const panda_file::File *file);

private:
    static std::map<const std::string, PCodeItem *> aexItemMap_;
    static ark::os::memory::Mutex jitItemLock_;
    static ark::os::memory::Mutex aexItemLock_;

    NO_COPY_SEMANTIC(DebugInf);
    NO_MOVE_SEMANTIC(DebugInf);
};
}  // namespace ark::tooling

#endif  // PANDA_RUNTIME_INCLUDE_TOOLING_DEBUG_INF_H
