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

/*---
 desc: 09.05.Field_Ddeclarations
 name: 09.Classes/09.05.Field_Ddeclarations/class_with_optional_attributes_0005.ets
 tags:
 ---*/

class Teacher {
  name: string = "";
}

class Student {
  name?: string;
  age?: int;
  desc?: string;
  teacher?: Teacher
}

function main() : void {
  let s0 : Student = new Student();

  assertEQ( s0.name, undefined)
  assertEQ( s0.age, undefined)
  assertEQ( s0.desc, undefined)
  assertEQ( s0.teacher, undefined)
}
