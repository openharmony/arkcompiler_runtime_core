/**
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
// Before AOP:

class MyClass {
  handle(): double {
    let a: double = 3.14
    return a
  }
}

function main() {
  let c = new MyClass();
  console.log(c.handle());
}

// After AOP:

// class MyClass {
//   handle(): double {
//     let a: double = 3.14
//     let b: int = 0
//     let c: int = 1
//     if (b == c) {
//         return 0;
//     }
//     return a
//   }
// }

// function main() {
//   let c = new MyClass();
//   c.handle();
// }
