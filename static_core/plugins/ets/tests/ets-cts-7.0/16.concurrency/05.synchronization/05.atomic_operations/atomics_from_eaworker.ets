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
/*---
desc: Atomics.add from EAWorker returns the previous value and updates a shared {{ c.type }} element
---*/

import { EAWorker } from "std/core"

{%- if c.type == "Int8Array" or c.type == "Uint8Array" %}
let buf = new ArrayBuffer(1)
{%- elif c.type == "Int16Array" or c.type == "Uint16Array" %}
let buf = new ArrayBuffer(2)
{%- elif c.type == "Int32Array" or c.type == "Uint32Array" %}
let buf = new ArrayBuffer(4)
{%- endif %}
let view = new {{ c.type }}(buf)

function addInWorker(): long {
  return Atomics.add(view, 0, 41)
}

function main(): void {
  Atomics.store(view, 0, 1)

  const eaWorker = new EAWorker()
  eaWorker.start()

  const oldValue: long = eaWorker.run<long>(addInWorker).Await()

  arktest.assertEQ(1, oldValue)
  arktest.assertEQ(42, Atomics.load(view, 0))

  eaWorker.quit()
}
{%- endfor %}