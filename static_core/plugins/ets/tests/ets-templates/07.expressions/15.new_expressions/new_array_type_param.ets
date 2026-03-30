/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: >-
    Creating a resizable array with a type parameter using new T[size](element)
    syntax succeeds. Attempting to create a FixedArray of elements of a type
    parameter causes a compile-time error.
---*/

class A<T> {
    makeArray(element: T): T[] {
        return new T[3](element)
    }
}

function main(): int {
    let suite = new arktest.ArkTestsuite("New array with type parameter");

    suite.addTest("resizable array created with new T[3](element)", () => {
        let a = new A<number>();
        let arr = a.makeArray(5.0);
        arktest.assertEQ(arr.length, 3);
        arktest.assertEQ(arr[0], 5.0);
        arktest.assertEQ(arr[1], 5.0);
        arktest.assertEQ(arr[2], 5.0);
    });

    suite.addTest("resizable array created with string type param", () => {
        let a = new A<string>();
        let arr = a.makeArray("hello");
        arktest.assertEQ(arr.length, 3);
        arktest.assertEQ(arr[0], "hello");
    });

    return suite.run();
}
