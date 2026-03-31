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

{% for c in cases %}
/*---
desc: Unsigned right shift expression for enumeration with explicit type
---*/
enum TypedEnum: {{c.check_type}} { A = {{c.value_a}}, B = {{c.value_b}} }

function main(): void {
  let a: {{c.check_type}} = {{c.value_a}}
  let b: {{c.check_type}} = {{c.value_b}}
  let res1 = TypedEnum.A >>> TypedEnum.B
  let res2 = a >>> b
  arktest.assertEQ(res1, res2)
  arktest.assertEQ(TypedEnum.A >>> TypedEnum.B, a >>> b)
}

{% endfor %}
