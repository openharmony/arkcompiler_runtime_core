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
    AUTO-GENERATED message schema 1 of 4 (6000 message types with id/value
    accessors), modelling protobuf/IDL-generated code.
tags: [not-a-test, compile-only]
name: lib_pb1
arktsconfig: arktsconfig-pbcodec-lib1.json
es2panda_options: ['emit-metadata']
---*/
{% for i in range(6000) %}export class Msg1_{{ i }} {
  id: int = {{ i % 100 }}
  value: int = {{ i % 100 + 1 }}
  getId(): int { return this.id }
  getValue(): int { return this.value }
}
{% endfor %}
