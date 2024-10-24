/**
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
// Before AOP:

class MyClass {
    handle() {
        if (1 < 0) {
            print('abckit');
        } else {
            print('abckit');
        }
    }
}

function main() {
    let c = new MyClass();
    c.handle();
}

main();

// // After AOP:
//
// class MyClass {
//   handle() {
//     print('file: har_A/src/MyClass, function: MyClass.handle')
//     let t1 = Date.getTime()
//     // business logic
//     let t2 = Date.getTime()
//     print('Ellapsed time:')
//     print(t2 - t1)
//   }
// }
