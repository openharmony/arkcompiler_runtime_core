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

/*---
 desc: 09.05.Field_Ddeclarations
 name: 09.Classes/09.05.Field_Ddeclarations/class_with_optional_attributes_0001.ets
 tags:
 ---*/

class Student {
  name: string;
  age: int;
  desc?: string;

  constructor(name: string, age: int, desc: string) {
    this.name = name;
    this.age = age;
    this.desc = desc
  }

  constructor(name: string, age: int) {
    this.name = name;
    this.age = age;
  }
}

function main() : void {
  let s : Student = new Student("Klara", 18, "an excellent student");
  let s1 : Student = new Student("Karl", 18);

  assertEQ( s.name, "Klara")
  assertEQ( s.age, 18)
  assertEQ( s.desc, "an excellent student")
  assertEQ( s1.name, "Karl")
  assertEQ( s1.age, 18)
  assertEQ( s1.desc, undefined)
}
