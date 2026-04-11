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

#ifndef PANDA_RUNTIME_TOOLING_BACKTRACE_LOCAL_STACKTRACE_H
#define PANDA_RUNTIME_TOOLING_BACKTRACE_LOCAL_STACKTRACE_H

#include "libarkbase/os/mutex.h"
#include "libarkfile/file.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/runtime.h"
#include "runtime/tooling/backtrace/base_defs.h"
#include "runtime/tooling/backtrace/symbol_extractor.h"

namespace ark::tooling {

struct __local_trace_ptr;
using LocalTrace = __local_trace_ptr *;

class LocalStackTrace final {
public:
    LocalStackTrace() = default;
    ~LocalStackTrace();

    NO_COPY_SEMANTIC(LocalStackTrace);
    NO_MOVE_SEMANTIC(LocalStackTrace);

    static LocalStackTrace *Create();
    static void Destroy(LocalStackTrace *trace);
    bool GetArkFrameInfo(uintptr_t pc, uintptr_t mapBase, uintptr_t loadOffset, Function *function);

private:
    bool InitializeMethodInfo(uintptr_t mapBase);
    const panda_file::File *FindArkpandaFile(uintptr_t mapBase);
    const std::vector<MethodInfo> FindMethodInfos(uintptr_t mapBase);
    const panda_file::File *FindArkPandaFileByMapBase(uintptr_t mapBase);
    void SetArkpandaFile(uintptr_t mapBase, const panda_file::File *pandafile);
    void SetMethodInfos(uintptr_t mapBase, std::vector<MethodInfo> infos);

    os::memory::RWLock pfMutex_;
    os::memory::RWLock infosMutex_;
    PandaUnorderedMap<uintptr_t, const panda_file::File *> arkPandaFiles_ GUARDED_BY(pfMutex_);
    PandaUnorderedMap<uintptr_t, std::vector<MethodInfo>> methodInfos_ GUARDED_BY(infosMutex_);
};

}  // namespace ark::tooling

#endif  // PANDA_RUNTIME_TOOLING_BACKTRACE_LOCAL_STACKTRACE_H
