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
desc: Assignment contexts allow the use of the identity conversion.
---*/

function main(): int {
  {%- for t in case['types'] %}
  let src_{{loop.index}}: {{t.type}} = {{t.expr|safe}};
  let dst_{{loop.index}}: {{t.type}} = src_{{loop.index}}
  if (src_{{loop.index}} {{t.op}} dst_{{loop.index}}) {
      return 1;
  }
  {% endfor %}
  return 0;
}
{% endfor %}
