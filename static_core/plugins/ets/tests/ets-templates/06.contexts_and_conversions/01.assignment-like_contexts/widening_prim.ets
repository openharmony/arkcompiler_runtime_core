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
desc: Assignment contexts allow the use of the widening primitive conversion.
params: Conversion from '{{case.src_type}}' to '{{case.dst_type}}'
---*/

function main(): int {
  let orig: {{case.src_type}} = {{case.src_val|safe}};
  let src: {{case.src_type}} = orig;
  let dst: {{case.dst_type}} = src;   // testing this assignment
  if (dst != {{case.dst_val}}) {
      return 1;
  }
  return 0;
}
{% endfor %}
