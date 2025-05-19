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
#include "runtime/include/panda_vm.h"
namespace ark {
bool ArkDebugNativeAPI::NotifyDebugMode([[maybe_unused]] int tid, [[maybe_unused]] int32_t instanceId,
                                        [[maybe_unused]] bool debugApp)
{
    LOG(INFO, DEBUGGER) << "ArkDebugNativeAPI::NotifyDebugMode, tid = " << tid << ", debugApp = " << debugApp
                        << ", instanceId = " << instanceId;
    if ((!debugApp) || (!Runtime::GetOptions().IsDebuggerEnable())) {
        return true;
    }

    auto loadRes = os::library_loader::Load(Runtime::GetOptions().GetDebuggerLibraryPath());
    if (!loadRes) {
        LOG(ERROR, DEBUGGER) << "Load library fail: " << Runtime::GetOptions().GetDebuggerLibraryPath() << " " << errno;
        return false;
    }
    os::library_loader::LibraryHandle handle = std::move(loadRes.Value());

    Runtime::GetCurrent()->SetDebugMode(true);

    using WaitForDebugger = void (*)();
    auto symOfWaitForDebugger = os::library_loader::ResolveSymbol(handle, "WaitForDebugger");
    if (!symOfWaitForDebugger) {
        LOG(ERROR, DEBUGGER) << "Resolve symbol WaitForDebugger fail: " << symOfWaitForDebugger.Error().ToString();
        return false;
    }
    reinterpret_cast<WaitForDebugger>(symOfWaitForDebugger.Value())();

    return true;
}

bool ArkDebugNativeAPI::StopDebugger()
{
    LOG(INFO, DEBUGGER) << "ArkDebugNativeAPI::StopDebugger";

    ark::Runtime::GetCurrent()->SetDebugMode(false);
    return true;
}

bool ArkDebugNativeAPI::StartDebuggerForSocketPair([[maybe_unused]] int tid, [[maybe_unused]] int socketfd)
{
    LOG(INFO, DEBUGGER) << "ArkDebugNativeAPI::StartDebuggerForSocketPair, tid = " << tid;

    auto loadRes = os::library_loader::Load(Runtime::GetOptions().GetDebuggerLibraryPath());
    if (!loadRes) {
        LOG(ERROR, DEBUGGER) << "Load library fail: " << Runtime::GetOptions().GetDebuggerLibraryPath() << " " << errno;
        return false;
    }
    os::library_loader::LibraryHandle handle = std::move(loadRes.Value());

    using StartDebuggerForSocketpair = bool (*)(int, bool);
    auto sym = os::library_loader::ResolveSymbol(handle, "StartDebuggerForSocketpair");
    if (!sym) {
        LOG(ERROR, DEBUGGER) << "[StartDebuggerForSocketPair] Resolve symbol fail: " << sym.Error().ToString();
        return false;
    }
    bool breakOnStart = Runtime::GetOptions().IsDebuggerBreakOnStart();
    bool ret = reinterpret_cast<StartDebuggerForSocketpair>(sym.Value())(socketfd, breakOnStart);
    if (!ret) {
        // Reset the config
        ark::Runtime::GetCurrent()->SetDebugMode(false);
    }
    return ret;
}

}  // namespace ark
