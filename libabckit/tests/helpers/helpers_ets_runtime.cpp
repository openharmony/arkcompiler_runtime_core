/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "helpers_runtime.h"
#include "libabckit/src/logger.h"
#include "static_core/plugins/ets/runtime/napi/ets_napi.h"
#include "mem_manager/mem_manager.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifndef ABCKIT_ETS_STD_LIB
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ABCKIT_ETS_STD_LIB ""
#endif

namespace libabckit::test::helpers {

std::string ExecuteStaticAbc(const std::string &abcPath, const std::string &klassName, const std::string &methodName)
{
    LIBABCKIT_LOG(DEBUG) << "ExecuteStaticAbc: " << abcPath << ' ' << klassName << ' ' << methodName << '\n';

    EtsEnv *env {nullptr};
    EtsVM *vm {nullptr};

    std::vector<EtsVMOption> optionsVector;

    optionsVector = {
        {EtsOptionType::ETS_BOOT_FILE, ABCKIT_ETS_STD_LIB},   {EtsOptionType::ETS_BOOT_FILE, abcPath.c_str()},
        {EtsOptionType::ETS_ARK_FILE, abcPath.c_str()},       {EtsOptionType::ETS_INTERPRETER_TYPE, "cpp"},
        {EtsOptionType::ETS_GC_TRIGGER_TYPE, "heap-trigger"}, {EtsOptionType::ETS_NO_JIT, nullptr}};

    EtsVMInitArgs vmArgs;
    vmArgs.version = ETS_NAPI_VERSION_1_0;
    vmArgs.options = optionsVector.data();
    vmArgs.nOptions = static_cast<ets_int>(optionsVector.size());

    MemManager::Finalize();
    ETS_CreateVM(&vm, &env, &vmArgs);

    auto klass = env->FindClass(klassName.c_str());
    if (klass == nullptr) {
        LIBABCKIT_LOG(FATAL) << "No class " << klassName << " in " << abcPath << '\n';
    }
    auto method = env->GetStaticp_method(klass, methodName.c_str(), nullptr);
    if (klass == nullptr) {
        LIBABCKIT_LOG(FATAL) << "No method " << methodName << " in " << klassName << '\n';
    }

    std::stringstream ss;
    {
        CoutRedirect coutRedirect(ss.rdbuf());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        env->CallStaticVoidMethod(klass, method);
    }

    vm->DestroyEtsVM();

    LIBABCKIT_LOG(DEBUG) << "Output:\n" << ss.str() << '\n';

    return ss.str();
}

}  // namespace libabckit::test::helpers
