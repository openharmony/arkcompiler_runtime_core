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

{% yaml "atomic_basic.data.yaml" as yaml_file %}

{%- for c in yaml_file.cases %}
{%- if c.type == "Int8Array" or c.type == "Uint8Array" -%}
{%- set elementSize = 1 -%}
{%- elif c.type == "Int16Array" or c.type == "Uint16Array" -%}
{%- set elementSize = 2 -%}
{%- elif c.type == "Int32Array" or c.type == "Uint32Array" -%}
{%- set elementSize = 4 -%}
{%- elif c.type == "BigInt64Array" or c.type == "BigUint64Array" -%}
{%- set elementSize = 8 -%}
{%- endif -%}

/*---
desc: Atomics.sub subtracts a value from {{c.type}} and returns the previous value
---*/

function main() {
  let buf = new ArrayBuffer({{elementSize}})
  let view = new {{c.type}}(buf)

  Atomics.store(view, 0, 50)
  let oldValue = Atomics.sub(view, 0, 8)

  arktest.assertEQ(50, oldValue)
  arktest.assertEQ(42, Atomics.load(view, 0))
}

{%- endfor %}
