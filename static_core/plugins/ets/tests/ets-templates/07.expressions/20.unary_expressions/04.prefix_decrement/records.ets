/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
    The value 1 is subtracted from the value of the variable
    by using necessary conversions
---*/

function main(): void {
    let rn: Record<int, number> = {0: 13.0}
    arktest.assertEQ(--rn[0], 12.0)
    arktest.assertEQ(rn[0], 12.0)

    let rd: Record<int, double> = {0: 13.0}
    arktest.assertEQ(--rd[0], 12.0)
    arktest.assertEQ(rd[0], 12.0)

    let rf: Record<int, float> = {0: 13.0f}
    arktest.assertEQ(--rf[0], 12.0f)
    arktest.assertEQ(rf[0], 12.0f)

    let rl: Record<int, long> = {0: 13}
    arktest.assertEQ(--rl[0], 12)
    arktest.assertEQ(rl[0], 12)

    let ri: Record<int, int> = {0: 13}
    arktest.assertEQ(--ri[0], 12)
    arktest.assertEQ(ri[0], 12)

    let rs: Record<int, short> = {0: 13}
    arktest.assertEQ(--rs[0], 12)
    arktest.assertEQ(rs[0], 12)

    let rb: Record<int, byte> = {0: 13}
    arktest.assertEQ(--rb[0], 12)
    arktest.assertEQ(rb[0], 12)

}
