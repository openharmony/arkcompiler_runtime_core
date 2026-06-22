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
/*---
desc: >-
    Multi-library import: imports and USES 35000 distinct entities from 4 libraries.
    Exercises consumer-side import breadth / 16-bit foreign-index pressure (ceiling ~65534);
    this stays below it. Sum of all imported values is checked at runtime.
files: [lib_min0.ets, lib_min1.ets, lib_min2.ets, lib_min3.ets]
name: test_multi_import
arktsconfig: arktsconfig-mimax-import.json
es2panda_options: ['read-metadata']
expected_out:
    runtime:
        - "1732500"
---*/
{% set counts = [11000, 11000, 11000, 2000] %}{% for p in range(4) %}import { {% for i in range(counts[p]) %}Z{{ p }}_{{ i }}{% if not loop.last %}, {% endif %}{% endfor %} } from "@panda/min{{ p }}/lib_min{{ p }}"
{% endfor %}
function main(): void {
  let s = 0
{% for p in range(4) %}{% for i in range(counts[p]) %}  s += new Z{{ p }}_{{ i }}().v
{% endfor %}{% endfor %}  console.log(s)
}
