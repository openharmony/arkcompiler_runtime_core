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
desc: for-of loop correctly iterates all elements of iterable
assertion: CI-08-06-03-006-01
tags: []
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite('for-of iteration');
    suite.addTest('iterates_all_array_elements', () => {
        let values: number[] = [1, 2, 3, 4, 5];
        let sum = 0;
        for (const element of values) {
            sum += element;
        }
        arktest.assertEQ(sum, 15, 'for-of should iterate all elements');
    });
    return suite.run();
}
