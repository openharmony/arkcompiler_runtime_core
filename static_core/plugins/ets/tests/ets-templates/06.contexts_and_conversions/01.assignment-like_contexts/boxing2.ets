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
desc: Assignment contexts allow the use of a boxing conversion followed by a widening reference conversion
params: Conversion from '{{c.src_type}}' to '{{c.dst_type}}'
---*/

function main(): int {
  {% for v in c['values'] %}
  let src_{{loop.index}}: {{c.src_type}} = {{v|safe}};
  let dst_{{loop.index}}: {{c.dst_type}} = src_{{loop.index}};   // boxing in this assignment
  if (dst_{{loop.index}} == null) {
      return 1;
  }
  {% endfor %}
  return 0;
}
{% endfor %}
