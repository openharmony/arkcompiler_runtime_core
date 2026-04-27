/*---
Copyright (c) 2021-2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: Line separators
---*/

// PLEASE DO NOT EDIT!
// THIS FILE IS CONSTRUCTED BY HEX EDITOR WITH LINE SEPARATORS FROM 2.2.2
// function test(): int {
// \U+000A
// \U+000D
// \U+000Dlet a: int = 0\U+000A
// a = a + 1\U+2028
// return a\U+2029
//}

function main(): void {

arktest.assertEQ(test(), 1)
  }


function test(): int {

let a: int = 0
a = a + 1 return a }
