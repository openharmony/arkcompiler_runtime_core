/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

{% for fc in float_comparison %}

/*---
desc: Integer comparison {{fc.xvalue}} {{fc.op}} {{fc.yvalue}}
---*/

function main(): void {
  let x: {{fc.xtype}} = {{fc.xvalue}};
  let bx: {{fc.xboxed}} = new {{fc.xboxed}}({{fc.xvalue}} as {{fc.xtype}});
  let y: {{fc.ytype}} = {{fc.yvalue}};
  let by: {{fc.yboxed}} = new {{fc.yboxed}}({{fc.yvalue}} as {{fc.ytype}});
  assert (x {{fc.op}} y) == {{fc.result}};
  assert (bx {{fc.op}} by) == {{fc.result}};
  assert (x {{fc.op}} by) == {{fc.result}};
}

{% endfor %}
