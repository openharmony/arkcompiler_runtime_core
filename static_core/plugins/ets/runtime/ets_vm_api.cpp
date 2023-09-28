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

#include <iostream>
#include "ets_vm_api.h"
#include "ets_vm.h"
#include "generated/base_options.h"

#ifdef PANDA_TARGET_OHOS
#include <hilog/log.h>

static void LogPrint([[maybe_unused]] int id, int level, const char *component, [[maybe_unused]] const char *fmt,
                     const char *msg)
{
#ifdef PANDA_USE_OHOS_LOG
    constexpr static unsigned int ARK_DOMAIN = 0xD003F00;
    constexpr static auto TAG = "ArkEtsVm";
    constexpr static OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, ARK_DOMAIN, TAG};
    switch (level) {
        case panda::Logger::PandaLog2MobileLog::DEBUG:
            OHOS::HiviewDFX::HiLog::Debug(LABEL, "%{public}s", msg);
            break;
        case panda::Logger::PandaLog2MobileLog::INFO:
            OHOS::HiviewDFX::HiLog::Info(LABEL, "%{public}s", msg);
            break;
        case panda::Logger::PandaLog2MobileLog::ERROR:
            OHOS::HiviewDFX::HiLog::Error(LABEL, "%{public}s", msg);
            break;
        case panda::Logger::PandaLog2MobileLog::FATAL:
            OHOS::HiviewDFX::HiLog::Fatal(LABEL, "%{public}s", msg);
            break;
        case panda::Logger::PandaLog2MobileLog::WARN:
            OHOS::HiviewDFX::HiLog::Warn(LABEL, "%{public}s", msg);
            break;
        default:
            UNREACHABLE();
    }
#else
    switch (level) {
        case panda::Logger::PandaLog2MobileLog::DEBUG:
            OH_LOG_Print(LOG_APP, LOG_DEBUG, 0xFF00, "ArkEtsVm", "%s: %s", component, msg);
            break;
        case panda::Logger::PandaLog2MobileLog::INFO:
            OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkEtsVm", "%s: %s", component, msg);
            break;
        case panda::Logger::PandaLog2MobileLog::ERROR:
            OH_LOG_Print(LOG_APP, LOG_ERROR, 0xFF00, "ArkEtsVm", "%s: %s", component, msg);
            break;
        case panda::Logger::PandaLog2MobileLog::FATAL:
            OH_LOG_Print(LOG_APP, LOG_FATAL, 0xFF00, "ArkEtsVm", "%s: %s", component, msg);
            break;
        case panda::Logger::PandaLog2MobileLog::WARN:
            OH_LOG_Print(LOG_APP, LOG_WARN, 0xFF00, "ArkEtsVm", "%s: %s", component, msg);
            break;
        default:
            UNREACHABLE();
    }
#endif  // PANDA_USE_OHOS_LOG
}
#else
static void LogPrint([[maybe_unused]] int id, [[maybe_unused]] int level, [[maybe_unused]] const char *component,
                     [[maybe_unused]] const char *fmt, const char *msg)
{
    std::cerr << msg << "\n";
}
#endif  // PANDA_TARGET_OHOS

namespace panda::ets {

bool CreateRuntime(std::function<bool(base_options::Options *, RuntimeOptions *)> const &add_options)
{
    auto runtime_options = panda::RuntimeOptions("app");
    runtime_options.SetLoadRuntimes({"ets"});
#ifdef PANDA_TARGET_OHOS
    runtime_options.SetMobileLog(reinterpret_cast<void *>(LogPrint));
#endif

    panda::base_options::Options base_options("app");

    if (!add_options(&base_options, &runtime_options)) {
        return false;
    }

    panda::Logger::Initialize(base_options);

    LOG(DEBUG, RUNTIME) << "CreateRuntime";

    if (!panda::Runtime::Create(runtime_options)) {
        LOG(ERROR, RUNTIME) << "CreateRuntime: cannot create ets runtime";
        return false;
    }
    return true;
}

bool CreateRuntime(const std::string &stdlib_abc, const std::string &path_abc, const bool use_jit, const bool use_aot)
{
    auto add_opts = [&](base_options::Options *base_options, panda::RuntimeOptions *runtime_options) {
        base_options->SetLogLevel("info");
        runtime_options->SetBootPandaFiles({stdlib_abc, path_abc});
        runtime_options->SetPandaFiles({path_abc});
        runtime_options->SetGcTriggerType("heap-trigger");
        runtime_options->SetCompilerEnableJit(use_jit);
        runtime_options->SetEnableAn(use_aot);
        runtime_options->SetCoroutineJsMode(true);
        runtime_options->SetCoroutineImpl("stackful");
        return true;
    };
    return CreateRuntime(add_opts);
}

bool DestroyRuntime()
{
    LOG(DEBUG, RUNTIME) << "DestroyEtsRuntime: enter";
    auto res = panda::Runtime::Destroy();
    LOG(DEBUG, RUNTIME) << "DestroyEtsRuntime: result = " << res;
    return res;
}

std::pair<bool, int> ExecuteMain()
{
    auto runtime = panda::Runtime::GetCurrent();
    auto pf_path = runtime->GetPandaFiles()[0];
    LOG(INFO, RUNTIME) << "ExecuteEtsMain: '" << pf_path << "'";
    auto res = panda::Runtime::GetCurrent()->ExecutePandaFile(pf_path, "ETSGLOBAL::main", {});
    LOG(INFO, RUNTIME) << "ExecuteEtsMain: result = " << (res ? std::to_string(res.Value()) : "failed");
    return res ? std::make_pair(true, res.Value()) : std::make_pair(false, 1);
}

bool BindNative(const char *class_descriptor, const char *method_name, void *impl)
{
    auto *runtime = panda::Runtime::GetCurrent();
    auto *class_linker = runtime->GetClassLinker();
    auto *ext = class_linker->GetExtension(panda::SourceLanguage::ETS);
    auto *klass = ext->GetClass(panda::utf::CStringAsMutf8(class_descriptor));

    if (klass == nullptr) {
        panda::ManagedThread::GetCurrent()->ClearException();
        LOG(DEBUG, RUNTIME) << "BindNative: Cannot find class '" << class_descriptor << "'";
        return false;
    }

    auto *method = klass->GetDirectMethod(panda::utf::CStringAsMutf8(method_name));

    if (method == nullptr) {
        panda::ManagedThread::GetCurrent()->ClearException();
        LOG(DEBUG, RUNTIME) << "BindNative: Cannot find method '" << class_descriptor << "." << method_name << "'`";
        return false;
    }

    method->SetCompiledEntryPoint(impl);
    return true;
}

void LogError(const std::string &msg)
{
    LogPrint(0, panda::Logger::PandaLog2MobileLog::ERROR, nullptr, nullptr, msg.c_str());
}

}  // namespace panda::ets
