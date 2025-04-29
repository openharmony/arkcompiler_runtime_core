/*
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
#ifndef RUNTIME_INCLUDE_TAIHE_RUNTIME_HPP_
#define RUNTIME_INCLUDE_TAIHE_RUNTIME_HPP_
// NOLINTBEGIN

#include <taihe/string.hpp>

#include <ani.h>

namespace taihe {
void set_vm(ani_vm *vm);
ani_vm *get_vm();
ani_env *get_env();

class env_guard {
    ani_env *env;
    bool is_attached;

public:
    env_guard();
    ~env_guard();

    env_guard(env_guard const &) = delete;
    env_guard &operator=(env_guard const &) = delete;
    env_guard(env_guard &&) = delete;
    env_guard &operator=(env_guard &&) = delete;

    ani_env *get_env()
    {
        return env;
    }
};

void set_error(taihe::string_view msg);
void set_business_error(int32_t err_code, taihe::string_view msg);
void reset_error();
bool has_error();
}  // namespace taihe
// NOLINTEND
#endif  // RUNTIME_INCLUDE_TAIHE_RUNTIME_HPP_