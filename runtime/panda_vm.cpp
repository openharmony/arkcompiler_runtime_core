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

#include <cstdlib>

#include "runtime/include/runtime.h"
#include "runtime/include/runtime_options.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/mem/gc/reference-processor/reference_processor.h"

#include "libpandafile/file.h"

namespace panda {
namespace {
class DefaultDebuggerAgent : public LibraryAgent, public LibraryAgentLoader<DefaultDebuggerAgent, false> {
public:
    explicit DefaultDebuggerAgent(os::memory::Mutex &mutex)
        : LibraryAgent(mutex, PandaString(Runtime::GetOptions().GetDebuggerLibraryPath()), "StartDebugger",
                       "StopDebugger")
    {
    }

    bool Load() override
    {
        debug_session_ = Runtime::GetCurrent()->StartDebugSession();
        if (!debug_session_) {
            LOG(ERROR, RUNTIME) << "Could not start debug session";
            return false;
        }

        if (!LibraryAgent::Load()) {
            debug_session_.reset();
            return false;
        }

        return true;
    }

    bool Unload() override
    {
        auto result = LibraryAgent::Unload();
        debug_session_.reset();
        return result;
    }

private:
    bool CallLoadCallback(void *resolvedFunction) override
    {
        ASSERT(resolvedFunction);
        ASSERT(debug_session_);

        using StartDebuggerT = int (*)(uint32_t, tooling::DebugInterface *, void *);
        uint32_t port = Runtime::GetOptions().GetDebuggerPort();
        int res = reinterpret_cast<StartDebuggerT>(resolvedFunction)(port, &debug_session_->GetDebugger(), nullptr);
        if (res != 0) {
            LOG(ERROR, RUNTIME) << "'StartDebugger' has failed with " << res;
            return false;
        }

        return true;
    }

    bool CallUnloadCallback(void *resolvedFunction) override
    {
        ASSERT(resolvedFunction);

        using StopDebugger = int (*)();
        int res = reinterpret_cast<StopDebugger>(resolvedFunction)();
        if (res != 0) {
            LOG(ERROR, RUNTIME) << "'StopDebugger' has failed with " << res;
            return false;
        }

        return true;
    }

    Runtime::DebugSessionHandle debug_session_;
};
}  // namespace

/* static */
PandaVM *PandaVM::Create(Runtime *runtime, const RuntimeOptions &options, std::string_view runtime_type)
{
    LanguageContext ctx = runtime->GetLanguageContext(std::string(runtime_type));
    return ctx.CreateVM(runtime, options);
}

Expected<int, Runtime::Error> PandaVM::InvokeEntrypoint(Method *entrypoint, const std::vector<std::string> &args)
{
    if (!CheckEntrypointSignature(entrypoint)) {
        LOG(ERROR, RUNTIME) << "Method '" << entrypoint << "' has invalid signature";
        return Unexpected(Runtime::Error::INVALID_ENTRY_POINT);
    }
    Expected<int, Runtime::Error> ret = InvokeEntrypointImpl(entrypoint, args);
    ManagedThread *thread = ManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    bool has_exception = false;
    {
        ScopedManagedCodeThread s(thread);
        has_exception = thread->HasPendingException();
    }
    if (has_exception) {
        HandleUncaughtException();
        ret = EXIT_FAILURE;
    }

    return ret;
}

void PandaVM::HandleLdaStr(Frame *frame, BytecodeId string_id)
{
    coretypes::String *str =
        panda::Runtime::GetCurrent()->ResolveString(this, *frame->GetMethod(), string_id.AsFileId());
    frame->GetAccAsVReg().SetReference(str);
}

std::unique_ptr<const panda_file::File> PandaVM::OpenPandaFile(std::string_view location)
{
    return panda_file::OpenPandaFile(location);
}

coretypes::String *PandaVM::GetNonMovableString(const panda_file::File &pf, panda_file::File::EntityId id) const
{
    auto cached_string = GetStringTable()->GetInternalStringFast(pf, id);
    if (cached_string == nullptr) {
        return nullptr;
    }

    if (!GetHeapManager()->GetObjectAllocator().AsObjectAllocator()->IsObjectInNonMovableSpace(cached_string)) {
        return nullptr;
    }

    return cached_string;
}

bool PandaVM::ShouldEnableDebug()
{
    return !Runtime::GetOptions().GetDebuggerLibraryPath().empty() || Runtime::GetOptions().IsDebuggerEnable();
}

LoadableAgentHandle PandaVM::CreateDebuggerAgent()
{
    if (!Runtime::GetOptions().GetDebuggerLibraryPath().empty()) {
        return DefaultDebuggerAgent::LoadInstance();
    }

    return {};
}
}  // namespace panda
