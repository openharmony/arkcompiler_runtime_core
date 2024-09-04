/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

{% for c in cases %}
/*---
desc: Compile-time error if the chain of conversions contains two parameterized types that are not in the subtype relation.
params: Invalid conversion from '{{c.fromtype}}' to '{{c.totype}}'
tags: [negative, compile-only]
---*/

interface I {}
interface J extends I {}
class A implements I {}
class B extends A implements J {}
class C extends B {}
class R {}

class Clazz {
  public fld1: {{c.totype}};
  public fld2: {{c.totype}}|null;

  public constructor(p: {{c.totype}}) {
    this.fld1 = p;
  }

  public foo(p: {{c.totype}}): void {
    this.fld2 = p;
  }
}

function main(): int {
  let v: {{c.fromtype}} = {{c.fromval | safe}};
  let obj: Clazz = new Clazz(v);
  obj.foo(v);
  if (obj.fld1 !== v || obj.fld2 !== v) {
    return 1;
  }
  return 0;
}
{% endfor %}
