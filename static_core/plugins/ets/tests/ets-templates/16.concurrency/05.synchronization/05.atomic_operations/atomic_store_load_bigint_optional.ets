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

{% yaml "atomic_bigint.data.yaml" as yaml_file %}

{%- for c in yaml_file.cases %}
{%- set elementSize = 8 -%}

/*---
desc: Optional bigint coverage. Atomics.store writes a {{c.type}} value and Atomics.load reads it back
---*/

function main() {
  let buf = new ArrayBuffer({{elementSize}})
  let view = new {{c.type}}(buf)

  let stored = Atomics.store(view, 0, 77n)
  let loaded = Atomics.load(view, 0)

  arktest.assertEQ(77n, stored)
  arktest.assertEQ(77n, loaded)
}

{%- endfor %}
