/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
{%- for c in cases %}

/*---
desc: Different imported functions have the same name but different signatures.
files: ['./pkg_a/pkg_{{loop.index0}}.ets', './pkg_b/pkg_{{loop.index0}}.ets']
tags: [{{c.tags}}]
---*/

import * as PkgA from './pkg_a/pkg_{{loop.index0}}.ets'
import * as PkgB from './pkg_b/pkg_{{loop.index0}}.ets'

{{c.decl}}

function main(): int {
    let v1 = PkgB.foo{{loop.index0}}({{c.arg1}})
    let v2 = PkgA.foo{{loop.index0}}({{c.arg2}})
    return v1 == 1 && v2 == 2 ? 0 : 1
}
{%- endfor %}
