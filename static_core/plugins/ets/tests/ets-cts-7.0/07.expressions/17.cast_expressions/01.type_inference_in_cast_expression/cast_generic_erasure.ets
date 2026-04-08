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
    If target type is not preserved up to undefined by Type Erasure,
    then a compile-time warning occurs. The cast itself succeeds at runtime
    due to type erasure.
---*/

class X<out T> {}

function main(): int {
    let suite = new arktest.ArkTestsuite("Generic cast type erasure warning");

    suite.addTest("cast X<Object> to X<number> succeeds at runtime", () => {
        let p1 = new X<Object>();
        let p2 = p1 as X<number>; // intentional: testing cast expression operator (7.17)
        arktest.assertTrue(p2 instanceof X);
    });

    return suite.run();
}
