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

#include <ani.h>
#include <array>
#include <iostream>
#include <filesystem>
#include "plugins/ets/runtime/interop_js/jsbacktrace/backtrace.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/thread.h"

extern "C" {
namespace ark::ets::interop::js::testing {

static bool ReadMemTestFunc([[maybe_unused]] void *ctx, uintptr_t addr, uintptr_t *value, bool isRead32)
{
    if (addr == 0) {
        return false;
    }
    if (isRead32) {
        *value = *(reinterpret_cast<uint32_t *>(addr));
    } else {
        *value = *(reinterpret_cast<uintptr_t *>(addr));
    }
    return true;
}

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
// NOLINTNEXTLINE(readability-function-size)
static ani_int CallBacktrace([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object object)
{
    uintptr_t fileHeader = 0;
    const char *zipPath = std::getenv("ARK_ETS_INTEROP_JS_GTEST_ABC_PATH");
    if (zipPath == nullptr) {
        return 0;
    }
    std::unique_ptr<const ark::panda_file::File> pf = ark::panda_file::OpenPandaFileOrZip(zipPath);
    uint64_t abcSize = pf->GetHeader()->fileSize;
    std::vector<uint8_t> abcBuffer(abcSize);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::copy(pf->GetBase(), pf->GetBase() + abcSize, abcBuffer.begin());
    pf.reset();

    uintptr_t startFp = 0;
    std::vector<std::tuple<ark::Method *, size_t>> expectFrames;
    for (auto stack = ark::StackWalker::Create(ark::ManagedThread::GetCurrent()); stack.HasFrame(); stack.NextFrame()) {
        if (!stack.IsCFrame()) {
            if (startFp == 0) {
                startFp = reinterpret_cast<uintptr_t>(stack.GetFp());
                fileHeader = reinterpret_cast<uintptr_t>(stack.GetMethod()->GetPandaFile()->GetHeader());
            }
            expectFrames.emplace_back(std::make_tuple(stack.GetMethod(), stack.GetBytecodePc()));
        }
    }
    uintptr_t fp = startFp;
    uintptr_t sp = 0;
    uintptr_t pc = 0;
    uintptr_t bcCode = 0;
    std::vector<panda::ecmascript::JsFunction> frames;
    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
    while (fp != 0 && panda::ecmascript::Backtrace::EtsStepArk(nullptr, &ReadMemTestFunc, &fp, &sp, &pc, &bcCode)) {
        panda::ecmascript::JsFunction function;
        if (panda::ecmascript::Backtrace::EtsSymbolize(pc, fileHeader, bcCode, abcBuffer.data(), abcSize, &function) ==
            1) {
            frames.emplace_back(function);
        } else {
            return 0;
        }
    }
    if (expectFrames.size() == frames.size()) {
        for (size_t i = 0; i < expectFrames.size(); ++i) {
            ark::Method *method = std::get<0>(expectFrames[i]);
            size_t bytecodePC = std::get<1>(expectFrames[i]);
            std::stringstream ssFunction;
            std::stringstream ssSrc;
            ssFunction << method->GetClass()->GetName() << "." << method->GetName().data;
            ssSrc << frames[i].url << ":" << frames[i].line;
            if (ssFunction.str() != frames[i].functionName ||
                ssSrc.str() != ark::PandaStringToStd(method->GetLineNumberAndSourceFile(bytecodePC))) {
                return 0;
            }
        }
    }
    return 1;
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        std::cerr << "Unsupported ANI_VERSION_1" << std::endl;
        return ANI_ERROR;
    }

    static const char *className = "LTest/TestBacktrace;";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        std::cerr << "Not found '" << className << "'" << std::endl;
        return ANI_ERROR;
    }

    std::array methods = {
        ani_native_function {"callBacktrace", ":I", reinterpret_cast<void *>(CallBacktrace)},
    };

    if (ANI_OK != env->Class_BindNativeMethods(cls, methods.data(), methods.size())) {
        std::cerr << "Cannot bind native methods to '" << className << "'" << std::endl;
        return ANI_ERROR;
    };

    *result = ANI_VERSION_1;
    return ANI_OK;
}

}  // namespace ark::ets::interop::js::testing

}  // extern "C"
