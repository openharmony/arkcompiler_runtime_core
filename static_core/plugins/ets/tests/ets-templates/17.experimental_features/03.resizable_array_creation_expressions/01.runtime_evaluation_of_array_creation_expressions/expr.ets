/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: >-
    The dimension expressions are evaluated. The evaluation is performed left-to-right;
    if any expression evaluation completes abruptly, then the expressions to the right
    of it are not evaluated.
ark_options: ['--heap-size-limit=33554432', '--init-heap-size-limit=33554432']
timeout: 60
---*/

{{c.defs}}

function main(): int {
    {{c.use|indent}}
}
{%- endfor %}
