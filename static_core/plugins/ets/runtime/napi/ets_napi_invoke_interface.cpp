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

#include "plugins/ets/runtime/napi/ets_napi_invoke_interface.h"

#include <vector>

#include "generated/base_options.h"
#include "libpandabase/utils/logger.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_vm.h"

namespace panda::ets::napi {
static ets_int DestroyEtsVM(EtsVM *vm)
{
    if (vm == nullptr) {
        LOG(ERROR, RUNTIME) << "Cannot destroy eTS VM, it is null";
        return ETS_ERR;
    }

    auto runtime = Runtime::GetCurrent();
    if (runtime == nullptr) {
        LOG(ERROR, RUNTIME) << "Cannot destroy eTS VM, there is no current runtime";
        return ETS_ERR;
    }

    auto panda_vm = PandaEtsVM::FromEtsVM(vm);
    auto main_vm = runtime->GetPandaVM();
    if (panda_vm == main_vm) {
        Runtime::Destroy();
    } else {
        PandaEtsVM::Destroy(panda_vm);
    }

    return ETS_OK;
}

bool CheckVersionEtsNapi(ets_int version)
{
    return (version == ETS_NAPI_VERSION_1_0);
}

static ets_int GetEnv([[maybe_unused]] EtsVM *vm, EtsEnv **p_env, [[maybe_unused]] ets_int version)
{
    if (p_env == nullptr) {
        LOG(ERROR, RUNTIME) << "Cannot get environment, p_env is null";
        return ETS_ERR;
    }

    auto coroutine = EtsCoroutine::GetCurrent();
    if (coroutine == nullptr) {
        LOG(ERROR, RUNTIME) << "Cannot get environment, there is no current coroutine";
        return ETS_ERR;
    }

    if (!CheckVersionEtsNapi(version)) {
        *p_env = nullptr;
        return ETS_ERR_VER;
    }
    *p_env = coroutine->GetEtsNapiEnv();

    return ETS_OK;
}

static const struct ETS_InvokeInterface S_INVOKE_INTERFACE = {DestroyEtsVM, GetEnv};

const ETS_InvokeInterface *GetInvokeInterface()
{
    return &S_INVOKE_INTERFACE;
}

static EtsVMInitArgs S_DEFAULT_ARGS = {0, 0, nullptr};

extern "C" ets_int ETS_GetDefaultVMInitArgs(EtsVMInitArgs *vm_args)
{
    *vm_args = S_DEFAULT_ARGS;
    return ETS_OK;
}

extern "C" ets_int ETS_GetCreatedVMs(EtsVM **vm_buf, ets_size buf_len, ets_size *n_vms)
{
    if (n_vms == nullptr) {
        return ETS_ERR;
    }

    if (auto coroutine = EtsCoroutine::GetCurrent()) {
        *n_vms = 1;

        if (vm_buf == nullptr || buf_len < 1) {
            return ETS_ERR;
        }

        *vm_buf = coroutine->GetPandaVM();
    } else {
        *n_vms = 0;
    }

    return ETS_OK;
}

static void *LOG_PRINT_FUNCTION = nullptr;

static void EtsMobileLogPrint(int id, int level, const char *component, const char *fmt, const char *msg)
{
    int ets_level = EtsMobileLogggerLevel::EtsMobileLogLevelUnknown;
    switch (level) {
        case panda::Logger::PandaLog2MobileLog::UNKNOWN:
            ets_level = EtsMobileLogggerLevel::EtsMobileLogLevelUnknown;
            break;
        case panda::Logger::PandaLog2MobileLog::DEFAULT:
            ets_level = EtsMobileLogggerLevel::EtsMobileLogLevelDefault;
            break;
        case panda::Logger::PandaLog2MobileLog::VERBOSE:
            ets_level = EtsMobileLogggerLevel::EtsMobileLogLevelVerbose;
            break;
        case panda::Logger::PandaLog2MobileLog::DEBUG:
            ets_level = EtsMobileLogggerLevel::EtsMobileLogLevelDebug;
            break;
        case panda::Logger::PandaLog2MobileLog::INFO:
            ets_level = EtsMobileLogggerLevel::EtsMobileLogLevelInfo;
            break;
        case panda::Logger::PandaLog2MobileLog::WARN:
            ets_level = EtsMobileLogggerLevel::EtsMobileLogLevelWarn;
            break;
        case panda::Logger::PandaLog2MobileLog::ERROR:
            ets_level = EtsMobileLogggerLevel::EtsMobileLogLevelError;
            break;
        case panda::Logger::PandaLog2MobileLog::FATAL:
            ets_level = EtsMobileLogggerLevel::EtsMobileLogLevelFatal;
            break;
        case panda::Logger::PandaLog2MobileLog::SILENT:
            ets_level = EtsMobileLogggerLevel::EtsMobileLogLevelSilent;
            break;
        default:
            LOG(ERROR, RUNTIME) << "No such mobile log option";
    }

    auto log_print_callback = reinterpret_cast<FuncMobileLogPrint>(LOG_PRINT_FUNCTION);
    ASSERT(log_print_callback != nullptr);
    log_print_callback(id, ets_level, component, fmt, msg);
}

static bool ParseOptions(const EtsVMInitArgs *args, RuntimeOptions &runtime_options)
{
    std::vector<std::string> boot_panda_files;
    std::vector<std::string> aot_files;
    std::vector<std::string> ark_files;
    base_options::Options base_options("");

    runtime_options.SetLoadRuntimes({"ets"});

    Span<const EtsVMOption> options(args->options, args->nOptions);
    for (auto &o : options) {
        auto extra_str = reinterpret_cast<const char *>(o.extraInfo);

        switch (o.option) {
            case EtsOptionType::EtsLogLevel:
                base_options.SetLogLevel(extra_str);
                break;
            case EtsOptionType::EtsMobileLog:
                LOG_PRINT_FUNCTION = const_cast<void *>(o.extraInfo);
                runtime_options.SetMobileLog(reinterpret_cast<void *>(EtsMobileLogPrint));
                break;
            case EtsOptionType::EtsBootFile:
                boot_panda_files.emplace_back(extra_str);
                break;
            case EtsOptionType::EtsAotFile:
                aot_files.emplace_back(extra_str);
                break;
            case EtsOptionType::EtsArkFile:
                ark_files.emplace_back(extra_str);
                break;
            case EtsOptionType::EtsJit:
                runtime_options.SetCompilerEnableJit(true);
                break;
            case EtsOptionType::EtsNoJit:
                runtime_options.SetCompilerEnableJit(false);
                break;
            case EtsOptionType::EtsAot:
                runtime_options.SetEnableAn(true);
                break;
            case EtsOptionType::EtsNoAot:
                runtime_options.SetEnableAn(false);
                break;
            case EtsOptionType::EtsGcTriggerType:
                runtime_options.SetGcTriggerType(extra_str);
                break;
            case EtsOptionType::EtsGcType:
                runtime_options.SetGcType(extra_str);
                break;
            case EtsOptionType::EtsRunGcInPlace:
                runtime_options.SetRunGcInPlace(true);
                break;
            case EtsOptionType::EtsInterpreterType:
                runtime_options.SetInterpreterType(extra_str);
                break;
            default:
                LOG(ERROR, RUNTIME) << "No such option";
        }
    }

    Logger::Initialize(base_options);

    runtime_options.SetBootPandaFiles(boot_panda_files);
    runtime_options.SetAotFiles(aot_files);
    runtime_options.SetPandaFiles(ark_files);

    return true;
}

extern "C" ETS_EXPORT ets_int ETS_CreateVM(EtsVM **p_vm, EtsEnv **p_env, EtsVMInitArgs *vm_args)
{
    trace::ScopedTrace scoped_trace(__FUNCTION__);

    if (p_vm == nullptr || p_env == nullptr) {
        return ETS_ERR;
    }

    if (!CheckVersionEtsNapi(vm_args->version)) {
        LOG(ERROR, ETS_NAPI) << "Error: Unsupported Ets napi version = " << vm_args->version;
        return ETS_ERR_VER;
    }

    RuntimeOptions runtime_options;

    if (!ParseOptions(vm_args, runtime_options)) {
        return ETS_ERR;
    }

    if (!Runtime::Create(runtime_options)) {
        LOG(ERROR, RUNTIME) << "Cannot create runtime";
        return ETS_ERR;
    }

    ASSERT(Runtime::GetCurrent() != nullptr);

    auto coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);

    *p_vm = coroutine->GetPandaVM();
    ASSERT(*p_vm != nullptr);

    *p_env = coroutine->GetEtsNapiEnv();
    ASSERT(*p_env != nullptr);

    return ETS_OK;
}
}  // namespace panda::ets::napi
