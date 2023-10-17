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

#include "public.h"
#include "public_internal.h"
#include "runtime/include/mem/allocator.h"
#include "verification/config/config_load.h"
#include "verification/config/context/context.h"
#include "verification/cache/results_cache.h"
#include "verification/jobs/service.h"
#include "verification/jobs/job.h"

namespace panda::verifier {

Config *NewConfig()
{
    auto result = new Config;
    result->opts.Initialize();
    return result;
}

bool LoadConfigFile(Config *config, std::string_view config_file_name)
{
    return panda::verifier::config::LoadConfig(config, config_file_name);
}

void DestroyConfig(Config *config)
{
    config->opts.Destroy();
    delete config;
}

bool IsEnabled(Config const *config)
{
    ASSERT(config != nullptr);
    return config->opts.IsEnabled();
}

bool IsOnlyVerify(Config const *config)
{
    ASSERT(config != nullptr);
    return config->opts.IsOnlyVerify();
}

Service *CreateService(Config const *config, panda::mem::InternalAllocatorPtr allocator, ClassLinker *linker,
                       std::string const &cache_file_name)
{
    if (!cache_file_name.empty()) {
        VerificationResultCache::Initialize(cache_file_name);
    }
    auto res = allocator->New<Service>();
    res->config = config;
    res->class_linker = linker;
    res->allocator = allocator;
    res->config = config;
    res->verifier_service = allocator->New<VerifierService>(allocator, linker);
    res->verifier_service->Init();
    res->debug_ctx.SetConfig(&config->debug_cfg);
    return res;
}

void DestroyService(Service *service, bool update_cache_file)
{
    if (service == nullptr) {
        return;
    }
    VerificationResultCache::Destroy(update_cache_file);
    auto allocator = service->allocator;
    service->verifier_service->Destroy();
    allocator->Delete(service->verifier_service);
    allocator->Delete(service);
}

Config const *GetServiceConfig(Service const *service)
{
    return service->config;
}

// Do we really need this public/private distinction here?
inline Status ToPublic(VerificationResultCache::Status status)
{
    switch (status) {
        case VerificationResultCache::Status::OK:
            return Status::OK;
        case VerificationResultCache::Status::FAILED:
            return Status::FAILED;
        case VerificationResultCache::Status::UNKNOWN:
            return Status::UNKNOWN;
        default:
            UNREACHABLE();
    }
}

static void ReportStatus(Service const *service, Method const *method, std::string const &status)
{
    if (service->config->opts.show.status) {
        LOG(DEBUG, VERIFIER) << "Verification result for method " << method->GetFullName(true) << ": " << status;
    }
}

static bool VerifyClass(Class *clazz)
{
    auto *lang_plugin = plugin::GetLanguagePlugin(clazz->GetSourceLang());
    auto result = lang_plugin->CheckClass(clazz);
    if (!result.IsOk()) {
        LOG(ERROR, VERIFIER) << result.msg << " " << clazz->GetName();
        clazz->SetState(Class::State::ERRONEOUS);
        return false;
    }

    Class *parent = clazz->GetBase();
    if (parent != nullptr && parent->IsFinal()) {
        LOG(ERROR, VERIFIER) << "Cannot inherit from final class: " << clazz->GetName() << "->" << parent->GetName();
        clazz->SetState(Class::State::ERRONEOUS);
        return false;
    }

    clazz->SetState(Class::State::VERIFIED);
    return true;
}

Status Verify(Service *service, panda::Method *method, VerificationMode mode)
{
    using VStage = Method::VerificationStage;
    ASSERT(service != nullptr);

    if (method->IsIntrinsic()) {
        return Status::OK;
    }

    /*
     *  Races are possible where the same method gets simultaneously verified on different threads.
     *  But those are harmless, so we ignore them.
     */
    auto stage = method->GetVerificationStage();
    if (stage == VStage::VERIFIED_OK) {
        return Status::OK;
    }
    if (stage == VStage::VERIFIED_FAIL) {
        return Status::FAILED;
    }

    if (!IsEnabled(mode)) {
        ReportStatus(service, method, "SKIP");
        return Status::OK;
    }
    if (method->GetInstructions() == nullptr) {
        LOG(DEBUG, VERIFIER) << method->GetFullName(true) << " has no code, no meaningful verification";
        return Status::OK;
    }
    service->debug_ctx.AddMethod(*method, mode == VerificationMode::DEBUG);
    if (service->debug_ctx.SkipVerification(method->GetUniqId())) {
        LOG(DEBUG, VERIFIER) << "Skipping verification of '" << method->GetFullName()
                             << "' according to verifier configuration";
        return Status::OK;
    }

    auto uniq_id = method->GetUniqId();
    auto method_name = method->GetFullName();
    if (VerificationResultCache::Enabled()) {
        auto cached_status = ToPublic(VerificationResultCache::Check(uniq_id));
        if (cached_status != Status::UNKNOWN) {
            LOG(DEBUG, VERIFIER) << "Verification result of method '" << method_name
                                 << "' was cached: " << (cached_status == Status::OK ? "OK" : "FAIL");
            return cached_status;
        }
    }

    auto lang = method->GetClass()->GetSourceLang();
    auto *processor = service->verifier_service->GetProcessor(lang);

    if (processor == nullptr) {
        LOG(INFO, VERIFIER) << "Attempt to  verify " << method->GetFullName(true)
                            << "after service started shutdown, ignoring";
        return Status::FAILED;
    }

    auto const &method_options = service->config->opts.debug.GetMethodOptions();
    auto const &verif_method_options = method_options[method_name];
    LOG(DEBUG, VERIFIER) << "Verification config for '" << method_name << "': " << verif_method_options.GetName();
    LOG(INFO, VERIFIER) << "Started verification of method '" << method->GetFullName(true) << "'";

    // class verification can be called concurrently
    if ((method->GetClass()->GetState() < Class::State::VERIFIED) && !VerifyClass(method->GetClass())) {
        return Status::FAILED;
    }
    Job job {service, method, verif_method_options};
    bool result = job.DoChecks(processor->GetTypeSystem());

    LOG(DEBUG, VERIFIER) << "Verification result for '" << method_name << "': " << result;

    service->verifier_service->ReleaseProcessor(processor);

    VerificationResultCache::CacheResult(uniq_id, result);

    if (result) {
        method->SetVerificationStage(VStage::VERIFIED_OK);
        ReportStatus(service, method, "OK");
        return Status::OK;
    }
    method->SetVerificationStage(VStage::VERIFIED_FAIL);
    ReportStatus(service, method, "FAIL");
    return Status::FAILED;
}

}  // namespace panda::verifier
