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
desc: Safe access to field
---*/

class AInt {
    public x: Int = 1;
}

class AIntUninit {
    public x: int;
}

final class BWithX {
    public x: Int = 12345;
}

type TNullableB = BWithX | null

final class AWithBin {
    public bin: TNullableB = null

    constructor(initB: boolean = false) {
        if (!initB) return
        this.bin = new BWithX
    }
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Safe field access via ?.")

    suite.addTest("safe access returns field value when instance not null", () => {
        let instance: AInt | null = new AInt
        arktest.assertTrue(((value: AInt | null): Int | undefined => { return value?.x })(instance) instanceof Int)
    })

    suite.addTest("safe access returns undefined when instance is null", () => {
        let instance: AIntUninit | null = null;
        arktest.assertTrue(((value: AIntUninit | null): Int | undefined => { return value?.x })(instance) instanceof undefined)
    })

    suite.addTest("chained safe access through nested nullable fields", () => {
        let testX = new AWithBin;
        arktest.assertEQ(undefined, ((value: AWithBin | null): Int | undefined => { return value?.bin?.x })(testX))
        let testY = new AWithBin(false)
        arktest.assertEQ(undefined, ((value: AWithBin | null): Int | undefined => { return value?.bin?.x })(testY))
        let testZ = new AWithBin(true)
        arktest.assertEQ(12345, ((value: AWithBin | null): Int | undefined => { return value?.bin?.x })(testZ))
    })

    return suite.run()
}
