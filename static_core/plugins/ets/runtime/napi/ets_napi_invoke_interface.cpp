/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "napi/ets_napi.h"
#include "plugins/ets/runtime/napi/ets_napi_internal.h"
#include "plugins/ets/runtime/napi/ets_napi_invoke_interface.h"

#include <sstream>
#include <vector>

#include "generated/base_options.h"
#include "libpandabase/utils/logger.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_vm.h"

namespace ark::ets::napi {
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
    int etsLevel = EtsMobileLogggerLevel::ETS_MOBILE_LOG_LEVEL_UNKNOWN;
    switch (level) {
        case ark::Logger::PandaLog2MobileLog::UNKNOWN:
            etsLevel = EtsMobileLogggerLevel::ETS_MOBILE_LOG_LEVEL_UNKNOWN;
            break;
        case ark::Logger::PandaLog2MobileLog::DEFAULT:
            etsLevel = EtsMobileLogggerLevel::ETS_MOBILE_LOG_LEVEL_DEFAULT;
            break;
        case ark::Logger::PandaLog2MobileLog::VERBOSE:
            etsLevel = EtsMobileLogggerLevel::ETS_MOBILE_LOG_LEVEL_VERBOSE;
            break;
        case ark::Logger::PandaLog2MobileLog::DEBUG:
            etsLevel = EtsMobileLogggerLevel::ETS_MOBILE_LOG_LEVEL_DEBUG;
            break;
        case ark::Logger::PandaLog2MobileLog::INFO:
            etsLevel = EtsMobileLogggerLevel::ETS_MOBILE_LOG_LEVEL_INFO;
            break;
        case ark::Logger::PandaLog2MobileLog::WARN:
            etsLevel = EtsMobileLogggerLevel::ETS_MOBILE_LOG_LEVEL_WARN;
            break;
        case ark::Logger::PandaLog2MobileLog::ERROR:
            etsLevel = EtsMobileLogggerLevel::ETS_MOBILE_LOG_LEVEL_ERROR;
            break;
        case ark::Logger::PandaLog2MobileLog::FATAL:
            etsLevel = EtsMobileLogggerLevel::ETS_MOBILE_LOG_LEVEL_FATAL;
            break;
        case ark::Logger::PandaLog2MobileLog::SILENT:
            etsLevel = EtsMobileLogggerLevel::ETS_MOBILE_LOG_LEVEL_SILENT;
            break;
        default:
            LOG(ERROR, RUNTIME) << "No such mobile log option";
    }

    auto logPrintCallback = reinterpret_cast<FuncMobileLogPrint>(g_logPrintFunction);
    ASSERT(logPrintCallback != nullptr);
    logPrintCallback(id, etsLevel, component, fmt, msg);
}

struct ParsedOptions final {
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    std::vector<std::string> bootPandaFiles;
    std::vector<std::string> aotFiles;
    std::vector<std::string> arkFiles;
    base_options::Options baseOptions;
    // NOLINTEND(misc-non-private-member-variables-in-classes)

    explicit ParsedOptions(const std::string &exePath) : baseOptions(exePath) {}
};

static arg_list_t SplitString(const std::string &from, char delim)
{
    arg_list_t pathes;
    std::istringstream iss {from};
    std::string path;
    while (std::getline(iss, path, delim)) {
        pathes.emplace_back(path);
    }
    return pathes;
}

static void ParseOptionsHelper(RuntimeOptions &runtimeOptions, ParsedOptions &parsedOptions,
                               Span<const EtsVMOption> &options)
{
    for (auto &o : options) {
        auto extraStr = reinterpret_cast<const char *>(o.extraInfo);

        switch (o.option) {
            case EtsOptionType::ETS_LOG_LEVEL:
                parsedOptions.baseOptions.SetLogLevel(extraStr);
                break;
            case EtsOptionType::ETS_MOBILE_LOG:
                g_logPrintFunction = const_cast<void *>(o.extraInfo);
                runtimeOptions.SetMobileLog(reinterpret_cast<void *>(EtsMobileLogPrint));
                break;
            case EtsOptionType::ETS_BOOT_FILE:
                parsedOptions.bootPandaFiles.emplace_back(extraStr);
                break;
            case EtsOptionType::ETS_AOT_FILE:
                parsedOptions.aotFiles.emplace_back(extraStr);
                break;
            case EtsOptionType::ETS_ARK_FILE:
                parsedOptions.arkFiles.emplace_back(extraStr);
                break;
            case EtsOptionType::ETS_JIT:
                runtimeOptions.SetCompilerEnableJit(true);
                break;
            case EtsOptionType::ETS_NO_JIT:
                runtimeOptions.SetCompilerEnableJit(false);
                break;
            case EtsOptionType::ETS_AOT:
                runtimeOptions.SetEnableAn(true);
                break;
            case EtsOptionType::ETS_NO_AOT:
                runtimeOptions.SetEnableAn(false);
                break;
            case EtsOptionType::ETS_GC_TRIGGER_TYPE:
                runtimeOptions.SetGcTriggerType(extraStr);
                break;
            case EtsOptionType::ETS_GC_TYPE:
                runtimeOptions.SetGcType(extraStr);
                break;
            case EtsOptionType::ETS_RUN_GC_IN_PLACE:
                runtimeOptions.SetRunGcInPlace(true);
                break;
            case EtsOptionType::ETS_INTERPRETER_TYPE:
                runtimeOptions.SetInterpreterType(extraStr);
                break;
            case EtsOptionType::ETS_NATIVE_LIBRARY_PATH:
                runtimeOptions.SetNativeLibraryPath(SplitString(extraStr, ':'));
                break;
            default:
                LOG(ERROR, RUNTIME) << "No such option";
        }
    }
}

static bool ParseOptions(const EtsVMInitArgs *args, RuntimeOptions &runtimeOptions)
{
    ParsedOptions parsedOptions("");

    runtimeOptions.SetLoadRuntimes({"ets"});

    Span<const EtsVMOption> options(args->options, args->nOptions);
    ParseOptionsHelper(runtimeOptions, parsedOptions, options);

    Logger::Initialize(parsedOptions.baseOptions);

    runtimeOptions.SetBootPandaFiles(parsedOptions.bootPandaFiles);
    runtimeOptions.SetAotFiles(parsedOptions.aotFiles);
    runtimeOptions.SetPandaFiles(parsedOptions.arkFiles);

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

// We have separate shared library with ets_napi called libetsnative.so
// libetsnative.so contains same three ETS_* functions as libarkruntime.so
// libarktuntime.so exposes three _internal_ETS_* aliases
// And libetsnative.so ETS_* functions just forward calls to _internal_ETS_* functions
extern "C" ETS_EXPORT ets_int _internal_ETS_GetDefaultVMInitArgs(EtsVMInitArgs *vmArgs)
    __attribute__((alias("ETS_GetDefaultVMInitArgs")));
extern "C" ETS_EXPORT ets_int _internal_ETS_GetCreatedVMs(EtsVM **vmBuf, ets_size bufLen, ets_size *nVms)
    __attribute__((alias("ETS_GetCreatedVMs")));
extern "C" ETS_EXPORT ets_int _internal_ETS_CreateVM(EtsVM **pVm, EtsEnv **pEnv, EtsVMInitArgs *vmArgs)
    __attribute__((alias("ETS_CreateVM")));
}  // namespace ark::ets::napi
