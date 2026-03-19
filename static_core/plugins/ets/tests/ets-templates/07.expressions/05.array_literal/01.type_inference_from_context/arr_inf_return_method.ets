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
  Array literal type inferred from explicit return type of a method.
---*/

class DataProvider {
    getValues(): number[] {
        return [10, 20, 30]
    }

    getNames(): string[] {
        return ["alpha", "beta"]
    }

    getGrid(): number[][] {
        return [
            [1, 2],
            [3, 4]
        ]
    }
}

function main() {
    let dp = new DataProvider()

    let vals = dp.getValues()
    arktest.assertEQ(vals[0], 10)
    arktest.assertEQ(vals[2], 30)

    let names = dp.getNames()
    arktest.assertEQ(names[0], "alpha")

    let grid = dp.getGrid()
    arktest.assertEQ(grid[1][0], 3)
}
