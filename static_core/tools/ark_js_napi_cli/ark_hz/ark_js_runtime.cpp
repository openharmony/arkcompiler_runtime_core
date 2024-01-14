/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ark_hz/ark_js_runtime.h"
#include "ark_js_runtime.h"
#include "utils/utils.h"

#include "ecmascript/base/string_helper.h"
#include "ecmascript/napi/include/jsnapi.h"

#include <iostream>

namespace panda {

std::string GetHelper()
{
    return ecmascript::COMMON_HELP_HEAD_MSG + ecmascript::HELP_OPTION_MSG;
}

bool ArkJsRuntime::ProcessOptions(int argc, const char **argv, arg_list_t *filenames)
{
    if (argc < 2) {  // 2: at least have two arguments
        std::cerr << GetHelper();
        return false;
    }

    std::string files = argv[argc - 1];
    if (!ecmascript::base::StringHelper::EndsWith(files, ".abc")) {
        std::cerr << "The last argument must be abc file" << std::endl;
        std::cerr << GetHelper();
        return 1;
    }

    bool retOpt = options_.ParseCommand(argc - 1, argv);
    if (!retOpt) {
        std::cerr << GetHelper();
        return false;
    }

    *filenames = ecmascript::base::StringHelper::SplitString(files, ":");
    return true;
}

bool ArkJsRuntime::Init()
{
    if (vm_ != nullptr) {
        return true;
    }

    vm_ = JSNApi::CreateEcmaVM(options_);
    if (vm_ == nullptr) {
        std::cerr << "Cannot Create vm" << std::endl;
        return false;
    }

    engine_ = std::make_unique<ArkNativeEngine>(vm_, nullptr);

    engine_->SetGetAssetFunc(utils::GetAsset);
    engine_->SetCleanEnv([this] { JSNApi::DestroyJSVM(vm_); });

    uv_loop_init(&loop_);

    return true;
}

bool ArkJsRuntime::Execute(const std::string &filename)
{
    LocalScope scope(vm_);
    return JSNApi::Execute(vm_, filename, options_.GetEntryPoint());
}

void ArkJsRuntime::Loop()
{
    while (uv_run(&loop_, UV_RUN_NOWAIT)) {
        uv_run(engine_->GetUVLoop(), UV_RUN_NOWAIT);
    }
}

}  // namespace panda
