/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
    Built-in functions allow to convert strings representing numbers or any integer values into bigint ones.
---*/

function main(): int {

    let testSuite = new arktest.ArkTestsuite("Bigint_methods_and_constructor");

    {% for f in bigint_functions %}
    testSuite.addTest("method_{{loop.index}}", () => {
        let x: bigint = {{f.func}};
        arktest.assertEQ(x, {{f.res}})
    });
    {% endfor %}

    return testSuite.run();
}
