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
 name: 09.Classes/09.05.Field_Ddeclarations/class_with_optional_attributes_0003.ets
 tags:
 ---*/

class Teacher {
  name: string;

  constructor(name: string) {
    this.name = name;
  }
}

class Student {
  name: string;
  age: int;
  desc?: string;
  teacher?: Teacher

  constructor(name: string, age: int, desc: string, teacher: Teacher) {
    this.name = name;
    this.age = age;
    this.desc = desc
    this.teacher = teacher
  }

  constructor(name: string, age: int) {
    this.name = name;
    this.age = age;
  }

  constructor(name: string, age: int, desc: string) {
    this.name = name;
    this.age = age;
    this.desc = desc;
  }

  constructor(name: string, age: int, teacher: Teacher) {
    this.name = name;
    this.age = age;
    this.teacher = teacher;
  }
}

function main() : void {
  let s0 : Student = new Student("Klara", 18, "an excellent student", new Teacher("Anna"));
  let s1 : Student = new Student("Klara", 18);
  let s2 : Student = new Student("Klara", 18, "an excellent student");
  let s3 : Student = new Student("Klara", 18,  new Teacher("Anna"));

  assertEQ( s0.name, "Klara")
  assertEQ( s0.age, 18)
  assertEQ( s0.desc, "an excellent student")
  assertEQ( s0.teacher?.name, "Anna")

  assertEQ( s1.name, "Klara")
  assertEQ( s1.age, 18)
  assertEQ( s1.desc, undefined)
  assertTrue( s1.teacher?.name ==undefined)

  assertEQ( s2.name, "Klara")
  assertEQ( s2.age, 18)
  assertEQ( s2.desc, "an excellent student")
  assertEQ( s2.teacher?.name, undefined)

  assertEQ( s3.name, "Klara")
  assertEQ( s3.age, 18)
  assertEQ( s3.desc, undefined)
  assertEQ( s3.teacher?.name, "Anna")
}
