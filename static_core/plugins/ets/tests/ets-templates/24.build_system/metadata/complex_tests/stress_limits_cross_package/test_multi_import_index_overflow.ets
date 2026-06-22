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
    AUTO-GENERATED protobuf/IDL codec registry: one module that instantiates every one of the
    24000 generated message types (across 4 schema libraries) and calls its getId()/getValue()
    accessors — the natural shape of a generated serialize-all/encode routine. This REFERENCES
    24000 classes + 48000 methods = 72000 distinct foreign index entries in ONE module,
    overflowing the 16-bit index region (MAX_INDEX_16=65534) → es2panda aborts (rc=134) with FATAL
    'Cannot add item index dependencies' (file_item_container.cpp:874). Mitigation in real codegen: split the codec
    across several modules so each
    references <=65534 entries. Listed in ets-cts-ignored.txt.
files: [lib_pb0.ets, lib_pb1.ets, lib_pb2.ets, lib_pb3.ets]
name: test_multi_import_index_overflow
arktsconfig: arktsconfig-pbcodec-import.json
expected_out:
    runtime:
        - "2400000"
---*/
{% for p in range(4) %}import { {% for i in range(6000) %}Msg{{ p }}_{{ i }}{% if not loop.last %}, {% endif %}{% endfor %} } from "@panda/schema{{ p }}/lib_pb{{ p }}"
{% endfor %}
function encodeAll(): int {
  let checksum = 0
{% for p in range(4) %}{% for i in range(6000) %}  { const m = new Msg{{ p }}_{{ i }}(); checksum += m.getId() + m.getValue() }
{% endfor %}{% endfor %}  return checksum
}

function main(): void {
  console.log(encodeAll())
}
