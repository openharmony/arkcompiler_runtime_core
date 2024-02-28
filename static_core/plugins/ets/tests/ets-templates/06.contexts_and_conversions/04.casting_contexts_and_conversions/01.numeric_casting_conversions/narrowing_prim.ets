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

{% for c in cases %}
/*---
desc: Casting contexts allow the use of the narrowing primitive conversion.
---*/

function main(): int {
  {%- for t in c['types'] %}
  let x{{loop.index}}: {{t.src_type}} = {{t.src_val|safe}};
  let y{{loop.index}}: {{t.dst_type}} = {{t.dst_val|safe}};
  if (x{{loop.index}} as {{t.dst_type}} != y{{loop.index}}) { // narrowing primitive casting here
      return 1;
  }
  {%- endfor %}
  return 0;
}
{% endfor %}
