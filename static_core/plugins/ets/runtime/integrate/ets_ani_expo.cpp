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
#include "plugins/ets/runtime/integrate/ets_ani_expo.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_napi_env.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/interop_js/interop_context_api.h"
#include "runtime/include/file_manager.h"

namespace ark::ets {
PANDA_PUBLIC_API void ETSAni::Prefork(ani_env *env)
{
    PandaEtsVM *vm = PandaEnv::FromAniEnv(env)->GetEtsVM();
    vm->PreZygoteFork();
}

PANDA_PUBLIC_API void ETSAni::Postfork(ani_env *env, const std::vector<ani_option> &options, bool postZygoteFork)
{
    [[maybe_unused]] EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);

    bool loadAppAotFile = false;
    std::string appAotFileName;
    for (auto &opt : options) {
        std::string option(opt.option);
        if (option == INTEROP_OPTION_PREFIX) {
#ifdef PANDA_ETS_INTEROP_JS
            if (!interop::js::CreateMainInteropContext(coroutine, opt.extra)) {
                LOG(ERROR, RUNTIME) << "Cannot create interop context";
            }
#endif
        } else if (option.rfind(AOT_FILE_OPTION_PREFIX, 0) == 0) {
            loadAppAotFile = true;
            appAotFileName = option.substr(AOT_FILE_OPTION_PREFIX.size());
        } else if (option == ENABLE_AN_OPTION) {
            TryLoadAotFileForBoot();
        } else {
            LOG(ERROR, RUNTIME) << "Unprocessed or invalid option parameter." << option;
        }
    }

    PandaEtsVM *vm = PandaEnv::FromAniEnv(env)->GetEtsVM();
    if (vm->GetLanguageContext().IsEnabledCHA()) {
        vm->GetRuntime()->GetClassLinker()->GetAotManager()->VerifyClassHierarchy();
    }

    if (loadAppAotFile) {
        LoadAotFileForApp(appAotFileName);
    }

    if (postZygoteFork) {
        vm->PostZygoteFork();
    }
}

void ETSAni::LoadAotFileForApp(std::string const &aotFileName)
{
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
    Runtime *runtime = Runtime::GetCurrent();
    ClassLinker *linker = runtime->GetClassLinker();
    const PandaVector<const panda_file::File *> bootPandaFiles = linker->GetBootPandaFiles();
    for (const auto *pf : bootPandaFiles) {
        const std::string location = pf->GetFilename();
        FileManager::TryLoadAnFileForLocation(location);
        const compiler::AotPandaFile *aotFile = linker->GetAotManager()->FindPandaFile(location);
        if (aotFile != nullptr) {
            pf->SetClassHashTable(aotFile->GetClassHashTable());
        }
    }
}
}  // namespace ark::ets
