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

#ifndef RUNTIME_RUNTIME_CONTROLLER_H_
#define RUNTIME_RUNTIME_CONTROLLER_H_

#include <string_view>

#include "libpandabase/macros.h"

namespace panda {

/**
 * For application related feature.
 */
class RuntimeController {
public:
    RuntimeController() = default;

    ~RuntimeController() = default;

    bool IsZidaneApp() const
    {
        return is_zidane_app_;
    }

    void SetZidaneApp(bool is_zidane_app)
    {
        is_zidane_app_ = is_zidane_app;
    }

    bool IsMultiFramework() const
    {
        return is_multi_framework_;
    }

    void SetMultiFramework(bool is_multi_framework)
    {
        is_multi_framework_ = is_multi_framework;
    }

    bool CanLoadPandaFileInternal(std::string_view real_path) const;

    bool CanLoadPandaFile(const std::string &path) const;

    NO_COPY_SEMANTIC(RuntimeController);
    NO_MOVE_SEMANTIC(RuntimeController);

private:
    bool is_zidane_app_ {false};
    bool is_multi_framework_ {false};
};

}  // namespace panda

#endif
