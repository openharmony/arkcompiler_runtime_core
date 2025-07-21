/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

const vals: bigint[] = [
    18446744073709551618n,
    9223372036854775810n,
    2n,
    0n,
    -2n,
    -9223372036854775810n,
    -18446744073709551618n
];

function testBigInt64ArraySet1() {
    let typedArray = new BigInt64Array(1)
    typedArray[0] = vals[0]
    assertEQ(typedArray[0], 2n)
}

function testBigInt64ArraySet2() {
    let typedArray = new BigInt64Array(1)
    typedArray[0] = vals[1]
    assertEQ(typedArray[0], -9223372036854775806n)
}

function testBigInt64ArraySet3() {
    let typedArray = new BigInt64Array(1)
    typedArray[0] = vals[2]
    assertEQ(typedArray[0], 2n)
}

function testBigInt64ArraySet4() {
    let typedArray = new BigInt64Array(1)
    typedArray[0] = vals[3]
    assertEQ(typedArray[0], 0n)
}

function testBigInt64ArraySet5() {
    let typedArray = new BigInt64Array(1)
    typedArray[0] = vals[4]
    assertEQ(typedArray[0], -2n)
}

function testBigInt64ArraySet6() {
    let typedArray = new BigInt64Array(1)
    typedArray[0] = vals[5]
    assertEQ(typedArray[0], 9223372036854775806n)
}

function testBigInt64ArraySet7() {
    let typedArray = new BigInt64Array(1)
    typedArray[0] = vals[6]
    assertEQ(typedArray[0], -2n)
}

function testBigInt64ArraySetWithVals() {
    let typedArray = new BigInt64Array(vals.length);
    typedArray.set(vals);
    assertEQ(typedArray[0], 2n);
    assertEQ(typedArray[1], -9223372036854775806n);
    assertEQ(typedArray[2], 2n);
    assertEQ(typedArray[3], 0n);
    assertEQ(typedArray[4], -2n);
    assertEQ(typedArray[5], 9223372036854775806n)
    assertEQ(typedArray[6], -2n)
}

function main(): int {
	const suite = new ArkTestsuite("test conversion between bigint and BigInt64Array")

	suite.addTest("convert 18446744073709551618n to BigInt64Array element", testBigInt64ArraySet1)
	suite.addTest("convert 9223372036854775810n to BigInt64Array element", testBigInt64ArraySet2)
	suite.addTest("convert 2n to BigInt64Array element", testBigInt64ArraySet3)
	suite.addTest("convert 0n to BigInt64Array element", testBigInt64ArraySet4)
	suite.addTest("convert -2n to BigInt64Array element", testBigInt64ArraySet5)
	suite.addTest("convert -9223372036854775810n to BigInt64Array element", testBigInt64ArraySet6)
	suite.addTest("convert -18446744073709551618n to BigInt64Array element", testBigInt64ArraySet7)
    suite.addTest("setBigInt64Array with vals", testBigInt64ArraySetWithVals)

	return suite.run()
}