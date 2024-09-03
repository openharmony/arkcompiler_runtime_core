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

{% for item in equality_primitive_01 %}

/*---
desc: {function: {{.item.type1}}_equal_{{.item.type2}}}
tags: []
---*/

function main(): void {
  let left: {{.item.type1}} = {{.item.value1}} as {{.item.type1}}
  let right: {{.item.type2}} = {{.item.value2}} as {{.item.type2}}
  let result1: boolean = left == right
  let result2: boolean = left === right
  assert result1 == result2 && result1 == {{.item.result}}
}

{%- endfor %}
