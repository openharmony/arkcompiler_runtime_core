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
class A {
<<<<<<< HEAD:static_core/plugins/ets/tests/ets-templates/06.contexts_and_conversions/05.implicit_conversions/06.char_to_string_conversions/comp-clss.ets
    fld: string = ""
=======
{%- for t in c['values'] %}
    fld{{loop.index}}: bigint = 0n
{%- endfor %}
>>>>>>> OpenHarmony_feature_20250328:static_core/plugins/ets/tests/ets-templates/03.types/15.type_bigint/bint_clss.ets
}
{%- for t in c['values'] %}
let prim{{loop.index}}: {{c.from_type}} = {{t.expr|safe}}
{%- endfor %}

function main() {
    let a: A = { {% for t in c['values'] %}fld{{loop.index}}: new BigInt(prim{{loop.index}}){{ ", " if not loop.last }}{% endfor %} }
{%- for t in c['values'] %}
    arktest.assertTrue( a.fld{{loop.index}} == {{t.val}})
{%- endfor %}
}
{%- endfor %}
