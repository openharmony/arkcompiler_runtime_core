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

{% for c in cases %}
/*---
desc: Casting contexts allow the use of an unboxing conversion.
---*/

function main(): int {
  {%- for t in c['types'] %}
  let s{{loop.index}}: {{t.type}} = new {{t.type}}({{t.val}});
  let d{{loop.index}}: {{t.type|lower}} = s{{loop.index}} as {{t.type|lower}};
  if (d{{loop.index}} != {{t.val}}) {
      return 1;
  }
  {%- endfor %}
  return 0;
}
{% endfor %}
