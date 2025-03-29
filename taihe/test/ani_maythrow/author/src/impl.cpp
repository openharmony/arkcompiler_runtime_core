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
#include <iostream>
#include <maythrow.impl.hpp>

#include "core/runtime.hpp"

int32_t maythrow_impl(int32_t a)
{
    if (a == 0) {
        taihe::core::set_error("some error happen");
        return -1;
    } else {
        return a + 10;
    }
}

TH_EXPORT_CPP_API_maythrow(maythrow_impl);
