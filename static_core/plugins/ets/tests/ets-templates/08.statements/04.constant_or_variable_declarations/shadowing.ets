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
desc: A local declaration can shadow another same-name declaration if any in the same surrounding scope.
---*/

function main(): int {
    let testsuite = new arktest.ArkTestsuite("local declaration shadowing");
    
    testsuite.addTest("Let declaration", () => {
        let item: int = 123;
        {
            let item: string = "123";
            {
                let item: double = 123.0;
                arktest.assertEQ(item, 123.0);
                arktest.assertTrue(item instanceof double);
            }
            arktest.assertEQ(item, "123");
            arktest.assertTrue(item instanceof string);
        }
        arktest.assertEQ(item, 123);
        arktest.assertTrue(item instanceof int);
    });

    testsuite.addTest("Const declaration", () => {
        const item: int = 123;
        {
            const item: string = "123";
            {
                const item: double = 123.0;
                arktest.assertEQ(item, 123.0);
                arktest.assertTrue(item instanceof double);
            }
            arktest.assertEQ(item, "123");
            arktest.assertTrue(item instanceof string);
        }
        arktest.assertEQ(item, 123);
        arktest.assertTrue(item instanceof int);
    });
    
    testsuite.addTest("Mixed declaration", () => {
        let item: int = 123;
        {
            const item: string = "123";
            {
                let item: double = 123.0;
                arktest.assertEQ(item, 123.0);
                arktest.assertTrue(item instanceof double);
            }
            arktest.assertEQ(item, "123");
            arktest.assertTrue(item instanceof string);
        }
        arktest.assertEQ(item, 123);
        arktest.assertTrue(item instanceof int);
    });

    return testsuite.run();
}
