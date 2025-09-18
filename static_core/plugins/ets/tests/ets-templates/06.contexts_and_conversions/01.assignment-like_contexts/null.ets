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

{% for case in cases %}
/*---
desc: >-
    A value of the null type (the null reference is the only such value) may be
    assigned to any reference type, resulting in a null reference of that type.
---*/

function main(): void {
  {%- for t in case['types'] %}
  let v_{{t}}: {{t}}|null = null;
  arktest.assertEQ(v_{{t}}, null)

  let a_{{t}}: ({{t}}[])|null = null;
  arktest.assertEQ(a_{{t}}, null)

  let f_{{t}}: ((p: {{t}}) => {{t}})|null = null;
  arktest.assertEQ(f_{{t}}, null)

  {% endfor %}
}
{% endfor %}
