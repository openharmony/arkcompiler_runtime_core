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

#include <ani.h>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "runtime/tooling/backtrace/backtrace.h"
#include "runtime/tooling/backtrace/base_defs.h"
#include "runtime/include/managed_thread.h"
#include "libarkbase/utils/logger.h"

namespace {

// Number of FP-chain steps from CallStepArkByNativeFrame's own frame to the
// interpreter handle frame. Using __builtin_frame_address(0) makes this value
// identical on x86_64 and ARM64:
//   #0 CallStepArkByNativeFrame (own frame, via __builtin_frame_address(0))
//   #1 AniEntryPoint
//   #2 InterpreterToCompiledCodeBridge
//   #3 HANDLE_FAST_CALL_SHORT_* (interpreter handle)
constexpr uint32_t FRAMES_TO_BRIDGE = 3U;

bool ReadMemTestFuncStatic([[maybe_unused]] void *ctx, uintptr_t addr, uintptr_t *value)
{
    if (addr == 0) {
        return false;
    }
    *value = *(reinterpret_cast<uintptr_t *>(addr));
    return true;
}

}  // namespace

ani_int CallStepArkByNativeFrame([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object object,
                                 [[maybe_unused]] ani_boolean useExtractor)
{
    uintptr_t fileHeader = 0;
    const char *zipPath = std::getenv("ARK_GTEST_ABC_PATH");
    if (zipPath == nullptr) {
        LOG(INFO, TOOLING) << "abc file is not found";
        return 0;
    }
    std::unique_ptr<const ark::panda_file::File> pf = ark::panda_file::OpenPandaFileOrZip(zipPath);
    if (pf == nullptr) {
        LOG(ERROR, TOOLING) << "Open abc file failed: " << zipPath;
        return 0;
    }
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

    // Use __builtin_frame_address(0) to get this function's own frame pointer.
    // This avoids an architecture-dependent extra frame that a separate function
    // call to get the current fp would introduce (x86_64: +1 vs ARM64: +0).
    uintptr_t currentFp = reinterpret_cast<uintptr_t>(__builtin_frame_address(0));
    if (!currentFp) {
        LOG(ERROR, TOOLING) << "The current arch is not supported.";
        return -1;
    }
    uintptr_t prevFp = currentFp;
    for (uint32_t i = 0; i < FRAMES_TO_BRIDGE; i++) {
        ReadMemTestFuncStatic(nullptr, currentFp, &prevFp);
        currentFp = prevFp;
    }

    uintptr_t fp = currentFp;
    uintptr_t sp = 0;
    uintptr_t pc = 0;
    bool isArkFrame = false;
    ark::tooling::StepFrameType frameType = ark::tooling::StepFrameType::ARK_MANAGED_FRAME;
    ark::tooling::ArkStepParam arkStepParam;
    arkStepParam.fp = &fp;
    arkStepParam.sp = &sp;
    arkStepParam.pc = &pc;
    arkStepParam.isArkFrame = &isArkFrame;
    arkStepParam.frameType = &frameType;
    arkStepParam.frameIndex = 0;
    uintptr_t loadOffset = 0;
    std::vector<ark::tooling::Function> frames;

    uintptr_t extractor = 0;
    if (useExtractor == ANI_TRUE) {
        extractor = ark::tooling::Backtrace::CreateArkSymbolExtractor();
    }

    while (*arkStepParam.frameType == ark::tooling::StepFrameType::ARK_MANAGED_FRAME) {
        if (*arkStepParam.fp == 0) {
            return 0;
        }
        // step stack ark frame
        if (!ark::tooling::Backtrace::StepArkByNativeFrame(nullptr, &ReadMemTestFuncStatic, &arkStepParam)) {
            LOG(INFO, TOOLING) << "Step stack ark frame failed";
            return 0;
        }
        arkStepParam.frameIndex++;
        if (*arkStepParam.frameType == ark::tooling::StepFrameType::NATIVE_FRAME) {
            break;
        }
        // symbolize stack ark frame
        ark::tooling::Function function;
        if (!ark::tooling::Backtrace::SymbolizeByNativeFrame(*arkStepParam.pc, fileHeader, loadOffset, abcBuffer.data(),
                                                             abcSize, &function, extractor)) {
            LOG(INFO, TOOLING) << "SymbolizeByNativeFrame stack ark frame failed";
            return 0;
        }
        frames.emplace_back(function);
    }

    if (useExtractor == ANI_TRUE) {
        ark::tooling::Backtrace::DestroyArkSymbolExtractor(extractor);
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
                LOG(INFO, TOOLING) << "Function name or source file mismatch";
                return 0;
            }
        }
    } else {
        return 0;
    }

    return frames.size();
}

ani_int ThrowErrorInNative([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object object)
{
    ani_class errCls;
    std::string msg = "Throw Error in native";
    char const *className = "std.core.Error";
    if (ANI_OK != env->FindClass(className, &errCls)) {
        std::cerr << "Not found '" << className << std::endl;
        return 0;
    }

    ani_method errCtor;
    if (ANI_OK != env->Class_FindMethod(errCls, "<ctor>", "C{std.core.String}C{std.core.ErrorOptions}:", &errCtor)) {
        std::cerr << "Get Ctor Failed '" << className << "'" << std::endl;
        return 0;
    }

    ani_string result_string {};
    env->String_NewUTF8(msg.c_str(), msg.size(), &result_string);

    ani_ref undefined;
    env->GetUndefined(&undefined);

    ani_error errObj;
    if (ANI_OK != env->Object_New(errCls, errCtor, reinterpret_cast<ani_object *>(&errObj), result_string, undefined)) {
        std::cerr << "Create Object Failed '" << className << "'" << std::endl;
        return 0;
    }

    if (env->ThrowError(errObj) != ANI_OK) {
        LOG(ERROR, TOOLING) << "Cannot ThrowError!";
        return 0;
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
    std::array methods = {
        ani_native_function {"callStepArkByNativeFrame", "z:i", reinterpret_cast<void *>(CallStepArkByNativeFrame)},
        ani_native_function {"throwErrorInNative", ":i", reinterpret_cast<void *>(ThrowErrorInNative)}};
    ani_module module;
    if (env->FindModule("call_backtrace_native", &module) != ANI_OK) {
        LOG(ERROR, TOOLING) << "Cannot find module!";
        return ANI_ERROR;
    }
    if (env->Module_BindNativeFunctions(module, methods.data(), methods.size()) != ANI_OK) {
        LOG(ERROR, TOOLING) << "Cannot bind native functions!";
        return ANI_ERROR;
    };

    *result = ANI_VERSION_1;
    return ANI_OK;
}
