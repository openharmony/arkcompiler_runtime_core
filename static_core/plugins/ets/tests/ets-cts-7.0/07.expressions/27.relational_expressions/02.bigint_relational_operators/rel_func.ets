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

/*---
desc: >-
    BigInt relational operations with function arguments.
---*/

function get_bigint(p: bigint): bigint { return p }
function get_operand(p: byte): byte { return p }
function get_operand(p: short): short { return p }
function get_operand(p: int): int { return p }
function get_operand(p: long): long { return p }
function get_operand(p: float): float { return p }
function get_operand(p: double): double { return p }

function main(): int {
    let tests = new arktest.ArkTestsuite('BigInt relational operations with function arguments')
{%- for c in cases %}
    tests.addTest(' {{c.desc}}. {{loop.index}}', () => {
        arktest.assertEQ(get_bigint({{c.b}}) {{c.op}} get_operand({{c.operand_val}}), {{c.expected_bool}})
    })
{% endfor %}
    return tests.run()
}