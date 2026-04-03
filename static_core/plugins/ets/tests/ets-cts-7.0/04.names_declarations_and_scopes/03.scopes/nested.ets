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
  Scopes of two names can overlap (e.g., when statements are nested),
  in this case access to the outer name is not accessible because the
  name with the innermost declaration takes precedence.
---*/

let x = 1

function get_func_x(): int {
    let x = 9
    return x
}

function get_global_x(): int {
    return x
}

function main(): int {
    let testsuite = new arktest.ArkTestsuite("Overlap scopes");
    
    testsuite.addTest("'Func' scope", () => {
        arktest.assertEQ( x, 1 )
        arktest.assertEQ( get_func_x(), 9 )
        arktest.assertEQ( x, 1 )
    });

    testsuite.addTest("'If' scope", () => {
        let x = 3
        if (true) {
            let x: int
            arktest.assertEQ( get_global_x(), 1 )
            arktest.assertEQ( x, 0 )
        }
        arktest.assertEQ( x, 3 )
    });

    return testsuite.run()
}
