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

#include "runtime/tooling/backtrace/local_stacktrace.h"

namespace ark::tooling {

LocalStackTrace *LocalStackTrace::Create()
{
    auto trace = new LocalStackTrace();
    return trace;
}

void LocalStackTrace::Destroy(LocalStackTrace *trace)
{
    if (trace == nullptr) {
        LOG(ERROR, RUNTIME) << "Destroy ark local stack trace failed, trace is nullptr.";
        return;
    }
    delete trace;
}

LocalStackTrace::~LocalStackTrace()
{
    {
        os::memory::WriteLockHolder rwlock(pfMutex_);
        arkPandaFiles_.clear();
    }
    {
        os::memory::WriteLockHolder rwlock(infosMutex_);
        methodInfos_.clear();
    }
}

bool LocalStackTrace::GetArkFrameInfo(uintptr_t pc, uintptr_t mapBase, uintptr_t loadOffset, Function *function)
{
    if (function == nullptr || pc == 0 || mapBase == 0) {
        LOG(ERROR, RUNTIME) << "parameter invalid!";
        return false;
    }

    const panda_file::File *pandaFile = FindArkpandaFile(mapBase);
    if (pandaFile == nullptr) {
        if (!InitializeMethodInfo(mapBase)) {
            LOG(ERROR, RUNTIME) << "Can not initialize method info.";
            return false;
        } else {
            pandaFile = FindArkpandaFile(mapBase);
        }
    }

    if (pandaFile == nullptr) {
        LOG(ERROR, RUNTIME) << "FindArkpandaFile failed, mapBase: 0x" << std::hex << mapBase;
        return false;
    }

    const std::vector<MethodInfo> methodInfos = FindMethodInfos(mapBase);
    if (methodInfos.empty()) {
        LOG(ERROR, RUNTIME) << "FindMethodInfos return nullptr, mapBase: 0x" << std::hex << mapBase;
        return false;
    }

    return SymbolizeByNativeFrameImpl(pc, mapBase, loadOffset, pandaFile, methodInfos, function);
}

bool LocalStackTrace::InitializeMethodInfo(uintptr_t mapBase)
{
    ASSERT(mapBase != 0);
    auto pandafile = FindArkPandaFileByMapBase(mapBase);
    if (pandafile == nullptr) {
        LOG(ERROR, RUNTIME) << "Find pandafile failed, mapBase: " << std::hex << mapBase;
        return false;
    }

    SetMethodInfos(mapBase, ReadAllMethodInfos(pandafile));
    SetArkpandaFile(mapBase, pandafile);
    return true;
}

const panda_file::File *LocalStackTrace::FindArkpandaFile(uintptr_t mapBase)
{
    ASSERT(mapBase != 0);
    os::memory::ReadLockHolder rlock(pfMutex_);
    auto iter = arkPandaFiles_.find(mapBase);
    if (iter == arkPandaFiles_.end()) {
        return nullptr;
    }
    return iter->second;
}

const panda_file::File *LocalStackTrace::FindArkPandaFileByMapBase(uintptr_t mapBase)
{
    ASSERT(mapBase != 0);
    const panda_file::File *result = nullptr;
    auto runtime = Runtime::GetCurrent();
    // After classlinker supports the pandafile unloading, the unloaded pandafile needs to be removed from cache.
    runtime->GetClassLinker()->EnumeratePandaFiles([&](const panda_file::File &file) -> bool {
        if (reinterpret_cast<uintptr_t>(file.GetHeader()) == mapBase) {
            result = &file;
            return false;
        }
        return true;
    });
    return result;
}

const std::vector<MethodInfo> LocalStackTrace::FindMethodInfos(uintptr_t mapBase)
{
    ASSERT(mapBase != 0);
    os::memory::ReadLockHolder rlock(infosMutex_);
    auto iter = methodInfos_.find(mapBase);
    if (iter == methodInfos_.end()) {
        return std::vector<MethodInfo> {};  // empty vector
    }
    return iter->second;
}

void LocalStackTrace::SetArkpandaFile(uintptr_t mapBase, const panda_file::File *pandafile)
{
    os::memory::WriteLockHolder rwlock(pfMutex_);
    arkPandaFiles_.emplace(mapBase, pandafile);
}

void LocalStackTrace::SetMethodInfos(uintptr_t mapBase, std::vector<MethodInfo> infos)
{
    os::memory::WriteLockHolder rwlock(infosMutex_);
    methodInfos_.emplace(mapBase, std::move(infos));
}
}  // namespace ark::tooling
