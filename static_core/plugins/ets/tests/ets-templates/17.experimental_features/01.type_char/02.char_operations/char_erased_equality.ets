/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
{%- for v in vals %}
/*---
desc: >-
    Equality of char against Any-typed operand via ets.equals or
    ets.strictequals bytecodes. Uses actual runtime types per spec
    7_expressions: char == numeric with same value is true.
---*/

function check(c: char, v: Any): boolean {
    return c {{ v.op }} v
}

function main() {
    const c: char = {{ v.v1 }}
    const val: Any = {{ v.v2 }}
    arktest.assertEQ(check(c, val), {{ v.expected }})
}
{%- endfor %}
