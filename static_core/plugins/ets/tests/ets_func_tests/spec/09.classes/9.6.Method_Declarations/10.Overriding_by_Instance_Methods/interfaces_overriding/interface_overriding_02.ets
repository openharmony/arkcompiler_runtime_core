/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
 desc: 10.06 Interface Method Declarations
 name: 10.interfaces/10.06.interface_method _declarations/10.06.02_inheritance_and_overriding/interface_overriding_02
 tags: []
 ---*/

interface I {
  foo(): I
  
  bar(): I
}

interface A extends I {
  foo(): A
}

interface AA extends A {
  foo(): AA

  bar(): AA
}


class B implements AA {
  private value: string = "";

  foo(): AA {
    this.value = "foo(): AA"
    return new B();
  }

  bar(): AA {
    this.value = "bar(): AA"
    return new B();
  }

  getValue(): string {
    return this.value;
  }
}

function main() {
  let b = new B()
  b.bar()
  assertEQ( b.getValue(), "bar(): AA")
}


