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

{% for c in cases %}
/*---
desc: The name of enumeration type can be indexed by a value of this enumeration type to get the value of the constant
---*/

enum Color { Red = {{ c.value1 }}, Green{{ c.value2 }}, Blue = {{ c.value3 }} }

function main() {
    let v = Color.values()
    {% for value in c.valuesArr %}
    assertEQ(v[{{ loop.index0 }}].valueOf(), {{ value }})
    {% endfor %}
}
{% endfor %}
