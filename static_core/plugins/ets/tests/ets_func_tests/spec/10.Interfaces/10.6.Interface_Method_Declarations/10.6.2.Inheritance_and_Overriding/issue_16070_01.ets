/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
 desc: 10.06.02 Inheritance and Overriding
 name: 10.interfaces/10.06.interface_method_declarations/10.06.02.inheritance_and_overriding/issue_16070_01
 tags: [negative, compile-only]
 ---*/

interface I {
  toString(): string
}

class Foo implements I {

  a: String
  
  constructor (a: String) {
    this.a = a
  }
  
  toString(): string {
    return "inside Foo"
  }
}

function main(): void {
  let foo = new Foo("any_string");
  assert foo.toString() == "inside Foo"
}