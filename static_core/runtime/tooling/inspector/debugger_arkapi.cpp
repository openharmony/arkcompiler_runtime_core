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

#include "debugger_arkapi.h"
#include <fstream>
#include <string_view>
#include "os/mutex.h"
#include "runtime/include/panda_vm.h"

namespace ark {
constexpr std::string_view ARK_DEBUGGER_LIB_PATH = "libark_inspector.z.so";
void *ArkDebugNativeAPI::gHybridDebuggerHandle_ = nullptr;
#ifdef PANDA_TARGET_WINDOWS
static void *Load(std::string_view libraryName)
{
    HMODULE module = LoadLibrary(libraryName.data());
    void *handle = reinterpret_cast<void *>(module);
    return handle;
}

static void *ResolveSymbol(void *handle, std::string_view symbol)
{
    HMODULE module = reinterpret_cast<HMODULE>(handle);
    void *addr = reinterpret_cast<void *>(GetProcAddress(module, symbol.data()));
    return addr;
}
#else  // UNIX_PLATFORM
static void *Load(std::string_view libraryName)
{
    void *handle = dlopen(libraryName.data(), RTLD_LAZY);
    return handle;
}

static void *ResolveSymbol(void *handle, std::string_view symbol)
{
    void *addr = dlsym(handle, symbol.data());
    return addr;
}
#endif

bool ArkDebugNativeAPI::NotifyDebugMode([[maybe_unused]] int tid, [[maybe_unused]] int32_t instanceId,
                                        [[maybe_unused]] bool debugApp, [[maybe_unused]] void *vm,
                                        [[maybe_unused]] DebuggerPostTask &debuggerPostTask)
{
    LOG(INFO, DEBUGGER) << "ArkDebugNativeAPI::NotifyDebugMode, tid = " << tid << ", debugApp = " << debugApp
                        << ", instanceId = " << instanceId;
    if ((!debugApp) || (!Runtime::GetOptions().IsDebuggerEnable())) {
        LOG(ERROR, DEBUGGER) << "Runtime::GetOptions().IsDebuggerEnable()" << Runtime::GetOptions().IsDebuggerEnable();
        return true;
    }
    if (!debuggerPostTask) {
        LOG(ERROR, DEBUGGER) << "ArkDebugNativeAPI::NotifyDebugMode, debuggerPostTask is nullptr";
        return false;
    }

    gHybridDebuggerHandle_ = Load(ARK_DEBUGGER_LIB_PATH);
    if (gHybridDebuggerHandle_ == nullptr) {
        LOG(ERROR, DEBUGGER) << "[NotifyDebugMode] gHybridDebuggerHandle_ load fail";
        return false;
    }
    Runtime::GetCurrent()->SetDebugMode(true);

    // store debugger postTask in inspector.
    using StoreDebuggerInfo = void (*)(int, void *, const DebuggerPostTask &);
    auto symOfStoreDebuggerInfo =
        reinterpret_cast<StoreDebuggerInfo>(ResolveSymbol(gHybridDebuggerHandle_, "StoreDebuggerInfo"));
    if (symOfStoreDebuggerInfo == nullptr) {
        LOG(ERROR, DEBUGGER) << "[NotifyDebugMode] Resolve StoreDebuggerInfo symbol fail: " << dlerror();
        return false;
    }
    symOfStoreDebuggerInfo(tid, vm, debuggerPostTask);

#ifndef PANDA_TARGET_ARM32
    // Initialize debugger
    using InitializeDebuggerForSocketpair = bool (*)(void *, bool);
    auto sym = reinterpret_cast<InitializeDebuggerForSocketpair>(
        ResolveSymbol(gHybridDebuggerHandle_, "InitializeDebuggerForSocketpair"));
    if (sym == nullptr) {
        LOG(ERROR, DEBUGGER) << "[NotifyDebugMode] InitializeDebuggerForSocketpair symbol fail: " << dlerror();
        return false;
    }
    if (!sym(vm, true)) {
        LOG(ERROR, DEBUGGER) << "[NotifyDebugMode] InitializeDebuggerForSocketpair fail";
        return false;
    }
#endif

    using WaitForDebugger = void (*)(void *);
    auto symOfWaitForDebugger =
        reinterpret_cast<WaitForDebugger>(ResolveSymbol(gHybridDebuggerHandle_, "WaitForDebugger"));
    if (symOfWaitForDebugger == nullptr) {
        LOG(ERROR, DEBUGGER) << "Resolve symbol WaitForDebugger fail: " << dlerror();
        return false;
    }
    symOfWaitForDebugger(vm);

    return true;
}

bool ArkDebugNativeAPI::StopDebugger([[maybe_unused]] void *vm)
{
    LOG(INFO, DEBUGGER) << "ArkDebugNativeAPI::StopDebugger";

    using StopDebug = void (*)(void *);
    auto sym = reinterpret_cast<StopDebug>(ResolveSymbol(gHybridDebuggerHandle_, "StopDebug"));
    if (sym == nullptr) {
        LOG(ERROR, DEBUGGER) << "g_initializeInspectorForStatic load error" << dlerror();
        return false;
    }

    sym(vm);
    ark::Runtime::GetCurrent()->SetDebugMode(false);
    return true;
}

bool ArkDebugNativeAPI::StartDebuggerForSocketPair([[maybe_unused]] int tid, [[maybe_unused]] int socketfd)
{
    LOG(INFO, DEBUGGER) << "ArkDebugNativeAPI::StartDebugForSocketPair, tid = " << tid << " socketfd is " << socketfd;
    if (!Runtime::GetOptions().IsDebuggerEnable()) {
        LOG(ERROR, DEBUGGER) << "Runtime::GetOptions().IsDebuggerEnable() " << Runtime::GetOptions().IsDebuggerEnable();
        return false;
    }
    using StartDebuggerForSocketpair = bool (*)(int, int, bool);
    auto sym =
        reinterpret_cast<StartDebuggerForSocketpair>(ResolveSymbol(gHybridDebuggerHandle_, "StartDebugForSocketpair"));
    if (sym == nullptr) {
        LOG(ERROR, DEBUGGER) << "g_initializeInspectorForStatic load error:%{public}s" << dlerror();
        return false;
    }
    bool ret = sym(tid, socketfd, true);
    if (!ret) {
        // Reset the config
        ark::Runtime::GetCurrent()->SetDebugMode(false);
    }
    return ret;
}

bool ArkDebugNativeAPI::IsDebugModeEnabled()
{
    LOG(INFO, DEBUGGER) << "ArkDebugNativeAPI::IsDebugModeEnabled is " << ark::Runtime::GetCurrent()->IsDebugMode();

    return ark::Runtime::GetCurrent()->IsDebugMode();
}
}  // namespace ark
