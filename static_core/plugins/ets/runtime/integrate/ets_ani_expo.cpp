/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include "plugins/ets/runtime/integrate/ets_ani_expo.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_ani_env.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/interop_js/interop_context_api.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "runtime/include/file_manager.h"
#include "runtime/include/thread_scopes.h"

namespace ark::ets {
PANDA_PUBLIC_API void ETSAni::Prefork(ani_env *env, [[maybe_unused]] void *napienv)
{
    PandaEtsVM *vm = PandaAniEnv::FromAniEnv(env)->GetEtsVM();
    ProcessTaskpoolWorker(true);
    vm->PreZygoteFork();
#ifdef PANDA_ETS_INTEROP_JS
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    if (!interop::js::CreateMainInteropContext(executionCtx, napienv)) {
        LOG(ERROR, RUNTIME) << "Cannot create interop context";
    }
#endif
}

PANDA_PUBLIC_API void ETSAni::Postfork(ani_env *env, const std::vector<ani_option> &options, bool postZygoteFork)
{
    PandaSmallVector<std::string> appAnFiles;
    for (auto &opt : options) {
        std::string option(opt.option);
        if (option.rfind(AOT_FILES_OPTION_PREFIX, 0) == 0) {
            std::string aotFilesRawInput = option.substr(AOT_FILES_OPTION_PREFIX.size());
            size_t start = 0;
            size_t end = aotFilesRawInput.find(':');
            while (end != std::string::npos) {
                appAnFiles.push_back(aotFilesRawInput.substr(start, end - start));
                start = end + 1;
                end = aotFilesRawInput.find(':', start);
            }
            appAnFiles.push_back(aotFilesRawInput.substr(start));
        } else if (option == ENABLE_AN_OPTION) {
            TryLoadAotFileForBoot();
        } else {
            LOG(ERROR, RUNTIME) << "Unprocessed or invalid option parameter." << option;
        }
    }

    PandaEtsVM *vm = PandaAniEnv::FromAniEnv(env)->GetEtsVM();
    if (vm->GetLanguageContext().IsEnabledCHA()) {
        vm->GetRuntime()->GetClassLinker()->GetAotManager()->VerifyClassHierarchy();
    }

    if (!appAnFiles.empty()) {
        for (const auto &path : appAnFiles) {
            LoadAotFileForApp(path);
        }
    }

    if (postZygoteFork) {
        vm->PostZygoteFork();
        ProcessTaskpoolWorker(false);
    }
}

void ETSAni::LoadAotFileForApp(std::string const &aotFileName)
{
    if (AotEscapeSignalHandler::IsEscapeSignalFlagExists()) {
        LOG(WARNING, AOT) << "LoadAotFileForApp: AOT mode has escaped and roll back to interpreter mode";
        return;
    }
    auto res = FileManager::LoadAnFile(aotFileName, true);
    if (!res) {
        LOG(ERROR, AOT) << "Failed to load AOT file: " << res.Error();
        return;
    }
    if (!res.Value()) {
        LOG(ERROR, AOT) << "Failed to load " << aotFileName << "with unkown reason";
        return;
    }
}

void ETSAni::TryLoadAotFileForBoot()
{
    if (AotEscapeSignalHandler::IsEscapeSignalFlagExists()) {
        LOG(WARNING, AOT) << "TryLoadAotFileForBoot: AOT mode has escaped and roll back to interpreter mode";
        return;
    }
    Runtime *runtime = Runtime::GetCurrent();
    ClassLinker *linker = runtime->GetClassLinker();
    const PandaVector<const panda_file::File *> bootPandaFiles = linker->GetBootPandaFiles();
    for (const auto *pf : bootPandaFiles) {
        const std::string location = pf->GetFilename();
        FileManager::TryLoadAnFileForLocation(location);
        const compiler::AotPandaFile *aotFile = linker->GetAotManager()->FindPandaFile(location);
        if (aotFile != nullptr) {
            pf->SetClassHashTable(aotFile->GetClassHashTable());
            // Issue 29288, temporary solution, need to support preload `an` files in hybridspawn nextly
            if (location == ETSSTDLIB_ABC) {
                linker->TryReLinkAotCodeForBoot(pf, aotFile, panda_file::SourceLang::ETS);
            }
        }
    }
}

void ETSAni::ProcessTaskpoolWorker(bool preFork)
{
    auto &taskpoolMode =
        Runtime::GetCurrent()->GetOptions().GetTaskpoolMode(plugins::LangToRuntimeType(panda_file::SourceLang::ETS));

    const char *taskpoolEAWorkerMode = "eaworker";
    if (taskpoolMode != taskpoolEAWorkerMode) {
        return;
    }

    if (preFork) {
        bool res = DestroyExclusiveWorkerForTaskpoolIfExists();
        LOG(INFO, COROUTINES) << "DestroyExclusiveWorkerForTaskpoolIfExists done: " << res;
    } else {
        bool res = PreCreateExclusiveWorkerForTaskpool();
        LOG(INFO, COROUTINES) << "PreCreateExclusiveWorkerForTaskpool done: " << res;
    }
}

bool ETSAni::PreCreateExclusiveWorkerForTaskpool()
{
    auto *runtime = Runtime::GetCurrent();
    auto *classLinker = runtime->GetClassLinker();
    ClassLinkerExtension *ext = classLinker->GetExtension(SourceLanguage::ETS);
    auto mutf8Name = reinterpret_cast<const uint8_t *>("Lstd/concurrency/taskpool;");
    auto *klass = ext->GetClass(mutf8Name);
    if (klass == nullptr) {
        LOG(ERROR, COROUTINES) << "Load taskpool failed in post zygote fork";
        return false;
    }
    auto method = ets::EtsClass::FromRuntimeClass(klass)->GetStaticMethod("initWorkerPool", ":V");
    if (method == nullptr) {
        LOG(ERROR, COROUTINES) << "Load taskpool initWorkerPool failed in post zygote fork";
        return false;
    }
    auto *mThread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread managedScope(mThread);
    method->GetPandaMethod()->Invoke(mThread, nullptr);
    return true;
}

bool ETSAni::DestroyExclusiveWorkerForTaskpoolIfExists()
{
    auto *runtime = Runtime::GetCurrent();
    auto *classLinker = runtime->GetClassLinker();
    ClassLinkerExtension *ext = classLinker->GetExtension(SourceLanguage::ETS);
    auto mutf8Name = reinterpret_cast<const uint8_t *>("Lstd/concurrency/taskpool;");
    auto *klass = ext->GetClass(mutf8Name);
    if (klass == nullptr) {
        LOG(ERROR, COROUTINES) << "Load taskpool failed in pre zygote fork";
        return false;
    }
    auto method = ets::EtsClass::FromRuntimeClass(klass)->GetStaticMethod("stopAllWorkers", ":V");
    if (method == nullptr) {
        LOG(ERROR, COROUTINES) << "Load taskpool::stopAllWorkers failed in pre zygote fork";
        return false;
    }
    auto *mThread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread managedScope(mThread);
    method->GetPandaMethod()->Invoke(mThread, nullptr);
    return true;
}

PANDA_PUBLIC_API void ETSAni::RegisterETSUncaughtExceptionHandler(ETSUncaughtExceptionCallback callback)
{
    Runtime::GetCurrent()->SetUncaughtExceptionCallback(std::move(callback));
}

PANDA_PUBLIC_API void ETSAni::HandleUncaughtException(ani_error aniError)
{
    Runtime::GetCurrent()->HandleUncaughtException(aniError);
}
}  // namespace ark::ets
