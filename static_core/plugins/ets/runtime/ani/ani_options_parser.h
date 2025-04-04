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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_ANI_OPTIONS_PARSER_H
#define PANDA_PLUGINS_ETS_RUNTIME_ANI_OPTIONS_PARSER_H

#include <string>
#include <vector>

#include "ani.h"
#ifdef PANDA_TARGET_OHOS
#include "plugins/ets/runtime/platform/ohos/logger.h"
#endif

namespace ark::ets::ani {

/**
 * @brief Class represents ANI options parser
 *
 * Class unites panda runtime options and ANI specific options parsing
 *
 */
class ANIOptionsParser {
public:
    using LoggerCallback = void (*)(FILE *stream, int level, const char *component, const char *message);

    ANIOptionsParser(size_t optionsSize, const ani_option *inputOptions)
        : optionsSize_(optionsSize), inputOptions_(inputOptions)
    {
        Parse();
    }

    const std::vector<std::string> &GetRuntimeOptions()
    {
        return runtimeOptions_;
    }

    const std::vector<ani_option> &GetANIOptions()
    {
        return aniSpecificOptions_;
    }

    bool IsInteropMode() const
    {
        return isInteropMode_;
    }

    void *GetInteropEnv() const
    {
        return interopEnv_;
    }

    LoggerCallback GetLoggerCallback()
    {
#ifdef PANDA_TARGET_OHOS
        return loggerCallback_ == nullptr ? ohos::DefaultLogger : loggerCallback_;
#else
        return loggerCallback_;
#endif
    }

private:
    void Parse()
    {
        const std::string depricatedPref = "--ext:--";
        const std::string prefix = "--ext:";

        for (size_t i = 0; i < optionsSize_; ++i) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            const ani_option opt = inputOptions_[i];
            std::string option(opt.option);

            if (option == "--logger") {
                loggerCallback_ = reinterpret_cast<LoggerCallback>(opt.extra);
                continue;
            }

            // NOTE(konstanting, #23205): this explicit comparison was requested by v.cherkashin
            // for better readability. To be refactored.
            if (option == "--ext:interop") {
                isInteropMode_ = true;
                interopEnv_ = opt.extra;
                continue;
            }

            // NOTE: need to skip forbidden options, such as "--load-runtimes"
            if (option.size() >= depricatedPref.size() && option.substr(0, depricatedPref.size()) == depricatedPref) {
                runtimeOptions_.push_back("--" + option.substr(depricatedPref.size()));
            } else if (option.size() >= prefix.size() && option.substr(0, prefix.size()) == prefix) {
                runtimeOptions_.push_back("--" + option.substr(prefix.size()));
            }
        }
    }

    void *interopEnv_ {nullptr};
    bool isInteropMode_ {false};
    LoggerCallback loggerCallback_ = nullptr;

    size_t optionsSize_;
    const ani_option *inputOptions_;
    std::vector<ani_option> aniSpecificOptions_;
    std::vector<std::string> runtimeOptions_;
};
}  // namespace ark::ets::ani

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_OPTIONS_PARSER_H
