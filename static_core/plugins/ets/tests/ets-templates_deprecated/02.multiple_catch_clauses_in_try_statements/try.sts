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

{% for t in cases %}
/*---
desc: >-
  Try statements run the block of code to handle exceptions and errors.
  Try statements can contain several catch clauses to handle exceptions and
  errors.
---*/

class Ex extends Exception {}
class Fx extends Exception {}
class X extends Error {}
class Y extends Error {}

let t1 = 1, t2 = 2  // global vars, never changed in tests

{{t.defs}}
function main(): int {
    {{t.use|indent}}
}
{% endfor %}
