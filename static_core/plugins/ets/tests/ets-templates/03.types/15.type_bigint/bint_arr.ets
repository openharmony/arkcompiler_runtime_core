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

{% for c in cases %}
/*---
desc: >-
    Numeric type values, by using a call to the standard library
    (see Standard Library) class BigInt methods or constructors.
---*/
<<<<<<< HEAD:static_core/plugins/ets/tests/ets-templates/07.expressions/25.equality_expressions/06.reference_equality_based_on_actual_type/union_equality_operators/eq.ets

function eq(p: {{c.type1}}, q: {{c.type2}}, ps: string, qs: string): string {
    if (p == q && q == p) {
        return ''
    } else {
        return '\n' + ps + ' == ' + qs + ', expected true, got false'
    }
}
function ne(p: {{c.type1}}, q: {{c.type2}}, ps: string, qs: string): string {
    if (p != q || q != p) {
        return '\n' + ps + ' != ' + qs + ', expected false, got true'
    } else {
        return ''
    }
}

{{c.decl}}

function main() {
    let failed: string = ''
{%- for p in c['case1'] %}
{%- for q in c['case2'] %}
    failed += eq({{p}}, {{q}}, "{{p|replace('\\', '\\\\')}}", "{{q|replace('\\', '\\\\')}}")
    failed += ne({{p}}, {{q}}, "{{p|replace('\\', '\\\\')}}", "{{q|replace('\\', '\\\\')}}")
{%- endfor %}
{%- endfor %}
    assertTrue(failed == '', 'Failed operations: ' + failed)
=======
{%- for t in c['values'] %}
let prim{{loop.index}}: {{c.from_type}} = {{t.expr|safe}}
{%- endfor %}

function main() {
    let a: bigint[] = [{% for t in c['values'] %}new BigInt(prim{{loop.index}}){{ ", " if not loop.last }}{% endfor %}]
{%- for t in c['values'] %}
    arktest.assertTrue( a[{{loop.index0}}] == {{t.val}})
{%- endfor %}
>>>>>>> OpenHarmony_feature_20250328:static_core/plugins/ets/tests/ets-templates/03.types/15.type_bigint/bint_arr.ets
}
{%- endfor %}
