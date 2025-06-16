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

function main(): int {
    let roundTestsuite = new ArkTestsuite("roundTest")
    roundTestsuite.addTest("round_normal", roundNormals)
    roundTestsuite.addTest("round_boundary", roundBoundaries)
    return roundTestsuite.run()
}

function roundNormals() {
    assertEQ(Math.round(0.), 0.)
    assertEQ(Math.round(1.), 1.)
    assertEQ(Math.round(-1.), -1.)
    assertEQ(Math.round(1.3), 1.)
    assertEQ(Math.round(-1.3), -1.)
    assertEQ(Math.round(1.5), 2)
    assertEQ(Math.round(-1.5), -1)
    assertEQ(Math.round(1.6), 2.)
    assertEQ(Math.round(-1.6), -2.)
    assertEQ(Math.round(0.1 + 0.4), 1.)
    assertEQ(Math.round(-0.1 - 0.4), -0.)
    assertEQ(Math.round(0.3), 0.)
    assertEQ(Math.round(-0.3), -0.)
}

function roundBoundaries() {
    assertEQ(Math.round(Double.POSITIVE_INFINITY), Double.POSITIVE_INFINITY, "round(Double.POSITIVE_INFINITY)")
    assertEQ(Math.round(Double.NEGATIVE_INFINITY), Double.NEGATIVE_INFINITY, "round(Double.NEGATIVE_INFINITY)")
    assertEQ(Double.toString(Math.round(Double.NaN)), Double.toString(Double.NaN), "Double.toString(Math.round(Double.NaN))")
    assertEQ(Double.toString(Math.round(-Double.NaN)), Double.toString(-Double.NaN), "Double.toString(Math.round(-Double.NaN))")
    assertEQ(Math.round(3.5000000000000000000000000001), 4., "round(3.5000000000000000000000000001)")       //Exceeds double storage precision, actual storage is 3.5
    assertEQ(Math.round(-3.5000000000000000000000000001), -3., "round(-3.5000000000000000000000000001)")    //Exceeds double storage precision, actual storage is -3.5
    assertEQ(Math.round(3.500000000000001), 4., "round(3.500000000000001)")
    assertEQ(Math.round(-3.500000000000001), -4., "round(-3.500000000000001)")
    assertEQ(Math.round(5.4999999999999999999999999999), 6., "round(5.4999999999999999999999999999)")       //Exceeds double storage precision, actual storage is 5.5
    assertEQ(Math.round(-5.4999999999999999999999999999), -5., "round(-5.4999999999999999999999999999)")    //Exceeds double storage precision, actual storage is -5.5
    assertEQ(Math.round(5.499999999999), 5., "round(5.499999999999)")
    assertEQ(Math.round(-5.499999999999), -5., "round(-5.499999999999)")
}
