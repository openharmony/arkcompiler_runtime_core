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
#ifndef TAIHE_RUNTIME_HPP
#define TAIHE_RUNTIME_HPP

#include <taihe/string.hpp>

#include <ani.h>

namespace taihe {
void set_env(ani_env *env);
ani_env *get_env();
void set_business_error(int32_t err_code, taihe::string_view msg);
void set_error(taihe::string_view msg);
void reset_error();
bool has_error();
}  // namespace taihe

#endif // TAIHE_RUNTIME_HPP
