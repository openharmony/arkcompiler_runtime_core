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

#include "mem/lock_config_helper.h"
#include "runtime/default_debugger_agent.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_options.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/mem/gc/reference-processor/reference_processor.h"

#include "libpandafile/file.h"

namespace panda {
/* static */
PandaVM *PandaVM::Create(Runtime *runtime, const RuntimeOptions &options, std::string_view runtime_type)
{
    LanguageContext ctx = runtime->GetLanguageContext(std::string(runtime_type));
    return ctx.CreateVM(runtime, options);
}

void PandaVM::VisitVmRoots(const GCRootVisitor &visitor)
{
    os::memory::LockHolder lock(mark_queue_lock_);
    for (ObjectHeader *obj : mark_queue_) {
        visitor(mem::GCRoot(mem::RootType::ROOT_VM, obj));
    }
}

void PandaVM::UpdateVmRefs()
{
    os::memory::LockHolder lock(mark_queue_lock_);
    // NOLINTNEXTLINE(modernize-loop-convert)
    for (auto it = mark_queue_.begin(); it != mark_queue_.end(); ++it) {
        if ((*it)->IsForwarded()) {
            *it = panda::mem::GetForwardAddress(*it);
        }
    }
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

// Intrusive GC test API
void PandaVM::MarkObject(ObjectHeader *obj)
{
    os::memory::LockHolder lock(mark_queue_lock_);
    mark_queue_.push_back(obj);
}

void PandaVM::IterateOverMarkQueue(const std::function<void(ObjectHeader *)> &visitor)
{
    os::memory::LockHolder lock(mark_queue_lock_);
    for (ObjectHeader *obj : mark_queue_) {
        visitor(obj);
    }
}

void PandaVM::ClearMarkQueue()
{
    os::memory::LockHolder lock(mark_queue_lock_);
    mark_queue_.clear();
}

LoadableAgentHandle PandaVM::CreateDebuggerAgent()
{
    if (!Runtime::GetOptions().GetDebuggerLibraryPath().empty()) {
        return DefaultDebuggerAgent::LoadInstance();
    }

    return {};
}

PandaString PandaVM::GetClassesFootprint() const
{
    ASSERT(GetLanguageContext().GetLanguageType() == LangTypeT::LANG_TYPE_STATIC);
    PandaVector<Class *> classes;
    auto class_linker = Runtime::GetCurrent()->GetClassLinker();
    class_linker->EnumerateClasses([&classes](Class *cls) {
        classes.push_back(cls);
        return true;
    });

    PandaVector<uint64_t> footprint_of_classes(classes.size(), 0U);
    GetHeapManager()->CountInstances(classes, true, footprint_of_classes.data());

    PandaMultiMap<uint64_t, Class *> footprint_to_class;
    for (size_t index = 0; index < classes.size(); ++index) {
        footprint_to_class.insert({footprint_of_classes[index], classes[index]});
    }

    PandaStringStream statistic;
    PandaMultiMap<uint64_t, Class *>::reverse_iterator rit;
    for (rit = footprint_to_class.rbegin(); rit != footprint_to_class.rend(); ++rit) {
        if (rit->first == 0U) {
            break;
        }
        auto clazz = rit->second;
        statistic << "class: " << clazz->GetName() << ", footprint - " << rit->first << std::endl;
    }
    return statistic.str();
}

}  // namespace panda
