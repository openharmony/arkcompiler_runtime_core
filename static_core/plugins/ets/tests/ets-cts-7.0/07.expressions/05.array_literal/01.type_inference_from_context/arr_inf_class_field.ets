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
  Array literal type inferred from the type of a class field.
---*/

class Config {
    values: number[] = [1, 2, 3]
    names: string[] = ["x", "y", "z"]
    matrix: number[][] = [
        [10, 20],
        [30, 40]
    ]
}

function main() {
    let cfg = new Config()

    arktest.assertEQ(cfg.values[0], 1)
    arktest.assertEQ(cfg.values[2], 3)

    arktest.assertEQ(cfg.names[1], "y")

    arktest.assertEQ(cfg.matrix[0][1], 20)
    arktest.assertEQ(cfg.matrix[1][0], 30)
}
