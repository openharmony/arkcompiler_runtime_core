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
 desc: 10.04.Interface_Members
 name: 10.Interfaces/10.04.Interface_Members/interface_with_optional_attributes_0004.ets
 tags:
 ---*/

interface IStudent {
  name?: string | undefined;
  age?: int | undefined;
  desc?: string | undefined;
  teacher?: Teacher | undefined;
}

class Teacher {
  name: string;

  constructor(name: string) {
    this.name = name;
  }
}

class Student implements IStudent {
  name?: string | undefined;
  age?: int | undefined;
  desc?: string | undefined;
  teacher?: Teacher | undefined;

  constructor(name: string, age: int, desc: string, teacher: Teacher) {
    this.name = name;
    this.age = age;
    this.desc = desc
    this.teacher = teacher
  }
}

function main() : void{
  let s0 : Student = new Student("Klara", 18, "an excellent student", new Teacher("Anna"));

  assertEQ( s0.name, "Klara")
  assertEQ( s0.age, 18)
  assertEQ( s0.desc, "an excellent student")
  assertEQ( s0.teacher?.name, "Anna")
}
