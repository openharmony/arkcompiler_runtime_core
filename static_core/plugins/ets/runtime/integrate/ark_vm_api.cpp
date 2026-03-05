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

#include "compiler/aot/aot_manager.h"
#include "libarkbase/macros.h"
#include "libarkbase/utils/logger.h"
#include "plugins/ets/runtime/integrate/ark_vm_api.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/main_worker_external_scheduler.h"
#include "runtime/include/runtime.h"

// CC-OFFNXT(G.FUN.02-CPP) project code style
extern "C" arkvm_status ARKVM_RegisterExternalScheduler(arkvm_external_scheduler_poster poster)
{
    if (poster == nullptr) {
        return ARKVM_INVALID_ARGS;
    }

    auto *mutator = ark::Mutator::GetCurrent();
    if (mutator == nullptr) {
        return ARKVM_INVALID_CONTEXT;
    }
    auto *coroutine = ark::ets::EtsCoroutine::GetCurrent();
    if (coroutine == nullptr || coroutine != coroutine->GetCoroutineManager()->GetMainThread()) {
        return ARKVM_INVALID_CONTEXT;
    }

    auto mainPoster = ark::MakePandaUnique<ark::ets::MainWorkerExternalScheduler>(poster);
    coroutine->GetWorker()->SetCallbackPoster(std::move(mainPoster));
    return ARKVM_OK;
}

// CC-OFFNXT(G.FUN.02-CPP) project code style
extern "C" arkvm_status ARKVM_RunScheduler(arkvm_schedule_mode mode)
{
    if (mode != ARKVM_SCHEDULER_RUN_ONCE) {
        return ARKVM_INVALID_ARGS;
    }

    auto *mutator = ark::Mutator::GetCurrent();
    if (mutator == nullptr) {
        return ARKVM_INVALID_CONTEXT;
    }
    auto *coroutine = ark::ets::EtsCoroutine::GetCurrent();
    if (coroutine == nullptr) {
        return ARKVM_INVALID_CONTEXT;
    }

    coroutine->GetCoroutineManager()->Schedule();
    return ARKVM_OK;
}

// CC-OFFNXT(G.FUN.02-CPP) project code style
extern "C" arkvm_status ARKVM_RegisterProfilePaths(const arkvm_profile_path_info *infos, size_t infoCount)
{
    if (infoCount == 0 || infos == nullptr) {
        return ARKVM_INVALID_ARGS;
    }

    auto *runtime = ark::Runtime::GetCurrent();
    if (runtime == nullptr) {
        return ARKVM_INVALID_CONTEXT;
    }

    auto *aotManager = runtime->GetClassLinker()->GetAotManager();
    bool hasInvalidArgs = false;
    ark::PandaVector<ark::compiler::AotManager::ProfilePathEntry> entries;
    entries.reserve(infoCount);
    for (size_t idx = 0; idx < infoCount; ++idx) {
        const auto &info = infos[idx];
        if (info.abcPath == nullptr || info.abcPath[0] == '\0') {
            hasInvalidArgs = true;
            continue;
        }
        if (info.curProfilePath == nullptr || info.curProfilePath[0] == '\0') {
            hasInvalidArgs = true;
            continue;
        }

        ark::PandaString baselinePath;
        if (info.baselineProfilePath != nullptr) {
            baselinePath = ark::PandaString(info.baselineProfilePath);
        }
        ark::compiler::ProfilePathInfo pathInfo {ark::PandaString(info.curProfilePath), std::move(baselinePath)};
        entries.emplace_back(ark::PandaString(info.abcPath), std::move(pathInfo));
    }
    if (hasInvalidArgs) {
        size_t invalidCount = infoCount - entries.size();
        LOG(DEBUG, RUNTIME) << "ARKVM_RegisterProfilePaths: valid entries count: " << entries.size()
                            << ", invalid entries count: " << invalidCount;
    }
    aotManager->RegisterProfilePaths(std::move(entries));
    if (hasInvalidArgs) {
        return entries.empty() ? ARKVM_INVALID_ARGS : ARKVM_PARTIAL_SUCCESS;
    }
    return ARKVM_OK;
}
