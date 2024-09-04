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

{% for case in cases %}
/*---
desc: Assignment contexts allow the use a widening reference conversion followed by an unboxing conversion.
params: Assignment of '{{case.type}}' to '{{case.ptype}}'
---*/

function foo<T extends {{case.type}}>(p: T): {{case.ptype}} {
  let src: T = p;
  let dst: {{case.ptype}} = src;
  return dst;
}

function main(): int {
  let res: {{case.ptype}} = foo<{{case.type}}>(new {{case.type}}({{case.val|safe}}));
  if (res != {{case.res|safe}}) {
      return 1;
  }
  return 0;
}
{% endfor %}
