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
 desc: 04.07.07 Return Type
 name: 04.names_declarations_and_scopes/04.07.function_declarations/04.07.07.return_type/issue_16080_03
 tags: 
 ---*/

class Foo {
  barNumber(): number {
    return 0
  }
}

function main(): void {
  let foo = new Foo()

  assertEQ( foo.barNumber(), 0)
}