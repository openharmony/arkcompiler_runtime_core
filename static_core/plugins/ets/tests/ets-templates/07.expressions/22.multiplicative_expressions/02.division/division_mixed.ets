/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: Division with mixed numeric types
params: {{c.x}}({{c.typex}}) / {{c.y}}({{c.typey}}) == {{c.result}}
---*/

function op(x: {{c.typex}}, y: {{c.typey}}): {{c.result_type}} {
  return x / y;
}

function main(): void {
  let x: {{c.typex}} = {{c.x}}
  let y: {{c.typey}} = {{c.y}}

  arktest.assertEQ(x / y, {{c.result}})
  arktest.assertEQ(op(x, y), {{c.result}})
}
{% endfor %}