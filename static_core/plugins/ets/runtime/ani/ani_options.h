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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_ANI_OPTIONS_H
#define PANDA_PLUGINS_ETS_RUNTIME_ANI_OPTIONS_H

#include "libpandabase/macros.h"
#include "libpandabase/utils/expected.h"
#include "utils/type_helpers.h"

#include <array>
#include <map>
#include <memory>
#include <string>
#include <variant>

namespace ark::ets::ani {

using LoggerCallback = void (*)(FILE *stream, int level, const char *component, const char *message);

class ANIOptions final {
public:
    enum class OptionKey {
        LOGGER,
#ifdef PANDA_ETS_INTEROP_JS
        INTEROP,
#endif  // PANDA_ETS_INTEROP_JS

        NUMBER_OF_ELEMENTS,
    };

    ANIOptions() = default;
    ~ANIOptions() = default;

    Expected<bool, std::string> SetOption(std::string_view key, std::string_view value, void *extra);

    LoggerCallback GetLoggerCallback() const;

#ifdef PANDA_ETS_INTEROP_JS
    bool IsInteropMode() const
    {
        const OptionValue *v = GetValue(OptionKey::INTEROP);
        return v != nullptr ? std::get<0>(v->value) : false;
    }

    void *GetInteropEnv() const
    {
        return GetExtra(OptionKey::INTEROP);
    }
#endif  // PANDA_ETS_INTEROP_JS

private:
    struct OptionValue {
        std::variant<bool, std::string> value;
        void *extra {};
    };

    struct OptionHandler {
        using Fn = Expected<std::unique_ptr<OptionValue>, std::string> (*)(std::string_view value, void *extra);

        OptionKey optionKey;
        Fn fn;
    };

    const OptionValue *GetValue(OptionKey key) const
    {
        return optionValues_[helpers::ToUnderlying(key)].get();
    }

    void *GetExtra(OptionKey key) const
    {
        const OptionValue *v = GetValue(key);
        return v != nullptr ? v->extra : nullptr;
    }

    const std::map<std::string_view, OptionHandler> &GetOptionsMap();

    std::array<std::unique_ptr<OptionValue>, helpers::ToUnderlying(OptionKey::NUMBER_OF_ELEMENTS)> optionValues_;

    NO_COPY_SEMANTIC(ANIOptions);
    NO_MOVE_SEMANTIC(ANIOptions);
};

}  // namespace ark::ets::ani

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_OPTIONS_H
