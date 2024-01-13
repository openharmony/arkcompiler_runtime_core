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

    auto pandaVm = PandaEtsVM::FromEtsVM(vm);
    auto mainVm = runtime->GetPandaVM();
    if (pandaVm == mainVm) {
        Runtime::Destroy();
    } else {
        PandaEtsVM::Destroy(pandaVm);
    }

    return ETS_OK;
}

bool CheckVersionEtsNapi(ets_int version)
{
    return (version == ETS_NAPI_VERSION_1_0);
}

static ets_int GetEnv([[maybe_unused]] EtsVM *vm, EtsEnv **pEnv, [[maybe_unused]] ets_int version)
{
    if (pEnv == nullptr) {
        LOG(ERROR, RUNTIME) << "Cannot get environment, p_env is null";
        return ETS_ERR;
    }

    auto coroutine = EtsCoroutine::GetCurrent();
    if (coroutine == nullptr) {
        LOG(ERROR, RUNTIME) << "Cannot get environment, there is no current coroutine";
        return ETS_ERR;
    }

    if (!CheckVersionEtsNapi(version)) {
        *pEnv = nullptr;
        return ETS_ERR_VER;
    }
    *pEnv = coroutine->GetEtsNapiEnv();

    return ETS_OK;
}

static const struct ETS_InvokeInterface S_INVOKE_INTERFACE = {DestroyEtsVM, GetEnv};

const ETS_InvokeInterface *GetInvokeInterface()
{
    return &S_INVOKE_INTERFACE;
}

static EtsVMInitArgs g_sDefaultArgs = {0, 0, nullptr};

extern "C" ets_int ETS_GetDefaultVMInitArgs(EtsVMInitArgs *vmArgs)
{
    *vmArgs = g_sDefaultArgs;
    return ETS_OK;
}

extern "C" ets_int ETS_GetCreatedVMs(EtsVM **vmBuf, ets_size bufLen, ets_size *nVms)
{
    if (nVms == nullptr) {
        return ETS_ERR;
    }

    if (auto coroutine = EtsCoroutine::GetCurrent()) {
        *nVms = 1;

        if (vmBuf == nullptr || bufLen < 1) {
            return ETS_ERR;
        }

        *vmBuf = coroutine->GetPandaVM();
    } else {
        *nVms = 0;
    }

    return ETS_OK;
}

static void *g_logPrintFunction = nullptr;

static void EtsMobileLogPrint(int id, int level, const char *component, const char *fmt, const char *msg)
{
    int etsLevel = EtsMobileLogggerLevel::EtsMobileLogLevelUnknown;
    switch (level) {
        case panda::Logger::PandaLog2MobileLog::UNKNOWN:
            etsLevel = EtsMobileLogggerLevel::EtsMobileLogLevelUnknown;
            break;
        case panda::Logger::PandaLog2MobileLog::DEFAULT:
            etsLevel = EtsMobileLogggerLevel::EtsMobileLogLevelDefault;
            break;
        case panda::Logger::PandaLog2MobileLog::VERBOSE:
            etsLevel = EtsMobileLogggerLevel::EtsMobileLogLevelVerbose;
            break;
        case panda::Logger::PandaLog2MobileLog::DEBUG:
            etsLevel = EtsMobileLogggerLevel::EtsMobileLogLevelDebug;
            break;
        case panda::Logger::PandaLog2MobileLog::INFO:
            etsLevel = EtsMobileLogggerLevel::EtsMobileLogLevelInfo;
            break;
        case panda::Logger::PandaLog2MobileLog::WARN:
            etsLevel = EtsMobileLogggerLevel::EtsMobileLogLevelWarn;
            break;
        case panda::Logger::PandaLog2MobileLog::ERROR:
            etsLevel = EtsMobileLogggerLevel::EtsMobileLogLevelError;
            break;
        case panda::Logger::PandaLog2MobileLog::FATAL:
            etsLevel = EtsMobileLogggerLevel::EtsMobileLogLevelFatal;
            break;
        case panda::Logger::PandaLog2MobileLog::SILENT:
            etsLevel = EtsMobileLogggerLevel::EtsMobileLogLevelSilent;
            break;
        default:
            LOG(ERROR, RUNTIME) << "No such mobile log option";
    }

    auto logPrintCallback = reinterpret_cast<FuncMobileLogPrint>(g_logPrintFunction);
    ASSERT(logPrintCallback != nullptr);
    logPrintCallback(id, etsLevel, component, fmt, msg);
}

static bool ParseOptions(const EtsVMInitArgs *args, RuntimeOptions &runtimeOptions)
{
    std::vector<std::string> bootPandaFiles;
    std::vector<std::string> aotFiles;
    std::vector<std::string> arkFiles;
    base_options::Options baseOptions("");

    runtimeOptions.SetLoadRuntimes({"ets"});

    Span<const EtsVMOption> options(args->options, args->nOptions);
    for (auto &o : options) {
        auto extraStr = reinterpret_cast<const char *>(o.extraInfo);

        switch (o.option) {
            case EtsOptionType::EtsLogLevel:
                baseOptions.SetLogLevel(extraStr);
                break;
            case EtsOptionType::EtsMobileLog:
                g_logPrintFunction = const_cast<void *>(o.extraInfo);
                runtimeOptions.SetMobileLog(reinterpret_cast<void *>(EtsMobileLogPrint));
                break;
            case EtsOptionType::EtsBootFile:
                bootPandaFiles.emplace_back(extraStr);
                break;
            case EtsOptionType::EtsAotFile:
                aotFiles.emplace_back(extraStr);
                break;
            case EtsOptionType::EtsArkFile:
                arkFiles.emplace_back(extraStr);
                break;
            case EtsOptionType::EtsJit:
                runtimeOptions.SetCompilerEnableJit(true);
                break;
            case EtsOptionType::EtsNoJit:
                runtimeOptions.SetCompilerEnableJit(false);
                break;
            case EtsOptionType::EtsAot:
                runtimeOptions.SetEnableAn(true);
                break;
            case EtsOptionType::EtsNoAot:
                runtimeOptions.SetEnableAn(false);
                break;
            case EtsOptionType::EtsGcTriggerType:
                runtimeOptions.SetGcTriggerType(extraStr);
                break;
            case EtsOptionType::EtsGcType:
                runtimeOptions.SetGcType(extraStr);
                break;
            case EtsOptionType::EtsRunGcInPlace:
                runtimeOptions.SetRunGcInPlace(true);
                break;
            case EtsOptionType::EtsInterpreterType:
                runtimeOptions.SetInterpreterType(extraStr);
                break;
            default:
                LOG(ERROR, RUNTIME) << "No such option";
        }
    }

    Logger::Initialize(baseOptions);

    runtimeOptions.SetBootPandaFiles(bootPandaFiles);
    runtimeOptions.SetAotFiles(aotFiles);
    runtimeOptions.SetPandaFiles(arkFiles);

    return true;
}

extern "C" ETS_EXPORT ets_int ETS_CreateVM(EtsVM **pVm, EtsEnv **pEnv, EtsVMInitArgs *vmArgs)
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);

    if (pVm == nullptr || pEnv == nullptr) {
        return ETS_ERR;
    }

    if (!CheckVersionEtsNapi(vmArgs->version)) {
        LOG(ERROR, ETS_NAPI) << "Error: Unsupported Ets napi version = " << vmArgs->version;
        return ETS_ERR_VER;
    }

    RuntimeOptions runtimeOptions;

    if (!ParseOptions(vmArgs, runtimeOptions)) {
        return ETS_ERR;
    }

    if (!Runtime::Create(runtimeOptions)) {
        LOG(ERROR, RUNTIME) << "Cannot create runtime";
        return ETS_ERR;
    }

    ASSERT(Runtime::GetCurrent() != nullptr);

    auto coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);

    *pVm = coroutine->GetPandaVM();
    ASSERT(*pVm != nullptr);

    *pEnv = coroutine->GetEtsNapiEnv();
    ASSERT(*pEnv != nullptr);

    return ETS_OK;
}
}  // namespace panda::ets::napi
