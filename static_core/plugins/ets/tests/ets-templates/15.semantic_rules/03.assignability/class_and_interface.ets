/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

{% for type in cases %}

/*---
desc: Type T1 is compatible with type T2 if there is an implicit conversion (see Implicit Conversions)
      that allows converting type T1 to type T2.
      Class does not implement interface.
tags: [negative, compile-only]
---*/

type TPL = [ number, string ]
type FUNC = () => boolean

interface I {
  i: {{type}};
}

class A {
  i: {{type}};
}

function main() {
  let i: I;
  i = new A();
}

{% endfor %}
