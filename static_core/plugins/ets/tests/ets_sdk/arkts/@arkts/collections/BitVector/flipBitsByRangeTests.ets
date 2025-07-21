/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { collections } from "@arkts.collections"
import { BusinessError } from "@ohos.base";

const OutOfBoundsErrorCodeId: number = 10200001;
const TypeErrorCodeId: number = 401;

function main() {
    const suite = new ArkTestsuite("BitVector flipBitsByRange API tests");

    suite.addTest("Test flipBitsByRange with valid range to verify it flips bits correctly (0→1, 1→0) for a small vector.", BitVector_flipBitsByRange_Func_001);
    suite.addTest("Test flipBitsByRange with negative toIndex to verify it throws appropriate error.", BitVector_flipBitsByRange_Func_002);
    suite.addTest("Test flipBitsByRange with both indices negative to verify it throws appropriate error.", BitVector_flipBitsByRange_Func_003);
    suite.addTest("Test flipBitsByRange with negative fromIndex to verify it throws appropriate error.", BitVector_flipBitsByRange_Func_004);
    suite.addTest("Test flipBitsByRange with non-integer fromIndex to verify it throws appropriate error.", BitVector_flipBitsByRange_Func_005);
    suite.addTest("Test flipBitsByRange on a 64-bit vector to verify it correctly flips the first half.", BitVector_flipBitsByRange_Func_006);
    suite.addTest("Test flipBitsByRange on a bit vector with initial capacity to verify correct flipping across the entire range.", BitVector_flipBitsByRange_Func_007);

    return suite.run();
}

/**
     * @tc.number    : BITVECTOR_TEST_064
     * @tc.name      : BitVector_flipBitsByRange_Func_001
     * @tc.desc      : Test flipBitsByRange with valid range to verify it flips bits correctly (0→1, 1→0) for a small vector.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
function BitVector_flipBitsByRange_Func_001() {
    let bitVector = new collections.BitVector(0);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(0);
    bitVector.push(1);
    bitVector.push(0);
    bitVector.flipBitsByRange(0, 4);
    assertEQ(bitVector[0], 0, "Bit at index 0 should be flipped to 0");
    assertEQ(bitVector[1], 0, "Bit at index 1 should be flipped to 0");
    assertEQ(bitVector[2], 1, "Bit at index 2 should be flipped to 1");
    assertEQ(bitVector[3], 0, "Bit at index 3 should be flipped to 0");
}

/**
     * @tc.number    : BITVECTOR_TEST_065
     * @tc.name      : BitVector_flipBitsByRange_Func_002
     * @tc.desc      : Test flipBitsByRange with negative toIndex to verify it throws appropriate error.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
function BitVector_flipBitsByRange_Func_002() {
    let bitVector = new collections.BitVector(0);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(0);
    bitVector.push(1);
    bitVector.push(0);
    bitVector.push(0);
    bitVector.push(1);
    bitVector.push(0);
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return e instanceof BusinessError &&
            ((e as BusinessError).code === OutOfBoundsErrorCodeId &&
                (e as BusinessError).message === `The value of "toIndex" is out of range. It must be >= 0 && <= 8. Received value is: -3`);
    }
    expectThrow(() => {
        bitVector.flipBitsByRange(8, -3);
    }, exceptionCheck);
}

/**
     * @tc.number    : BITVECTOR_TEST_066
     * @tc.name      : BitVector_flipBitsByRange_Func_003
     * @tc.desc      : Test flipBitsByRange with both indices negative to verify it throws appropriate error.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
function BitVector_flipBitsByRange_Func_003() {
    let bitVector = new collections.BitVector(0);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(0);
    bitVector.push(1);
    bitVector.push(0);
    bitVector.push(0);
    bitVector.push(1);
    bitVector.push(0);
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return e instanceof BusinessError &&
            ((e as BusinessError).code === OutOfBoundsErrorCodeId &&
                (e as BusinessError).message === `The value of "toIndex" is out of range. It must be >= 0 && <= 8. Received value is: -3`);
    }
    expectThrow(() => {
        bitVector.flipBitsByRange(-3, -3);
    }, exceptionCheck);
}

/**
     * @tc.number    : BITVECTOR_TEST_067
     * @tc.name      : BitVector_flipBitsByRange_Func_004
     * @tc.desc      : Test flipBitsByRange with negative fromIndex to verify it throws appropriate error.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
function BitVector_flipBitsByRange_Func_004() {
    let bitVector = new collections.BitVector(0);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(0);
    bitVector.push(1);
    bitVector.push(0);
    bitVector.push(0);
    bitVector.push(1);
    bitVector.push(0);
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return e instanceof BusinessError &&
            ((e as BusinessError).code === OutOfBoundsErrorCodeId &&
                (e as BusinessError).message === `The value of "fromIndex" is out of range. It must be >= 0 && <= 7. Received value is: -3`);
    }
    expectThrow(() => {
        bitVector.flipBitsByRange(-3, 8);
    }, exceptionCheck);
}

/**
     * @tc.number    : BITVECTOR_TEST_068
     * @tc.name      : BitVector_flipBitsByRange_Func_005
     * @tc.desc      : Test flipBitsByRange with non-integer fromIndex to verify it throws appropriate error.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
function BitVector_flipBitsByRange_Func_005() {
    let bitVector = new collections.BitVector(0);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(0);
    bitVector.push(1);
    bitVector.push(0);
    bitVector.push(0);
    bitVector.push(1);
    bitVector.push(0);
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return e instanceof BusinessError &&
            ((e as BusinessError).code === TypeErrorCodeId &&
                (e as BusinessError).message === `The type of "fromIndex" must be integer. Received value is: 3.11`);
    }
    expectThrow(() => {
        bitVector.flipBitsByRange(3.11, 8);
    }, exceptionCheck);
}

/**
     * @tc.number    : BITVECTOR_TEST_069
     * @tc.name      : BitVector_flipBitsByRange_Func_006
     * @tc.desc      : Test flipBitsByRange on a 64-bit vector to verify it correctly flips the first half.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
function BitVector_flipBitsByRange_Func_006() {
    let bitVector = new collections.BitVector(0);
    let bitVectorLength = 64;
    for (let i = 0; i < bitVectorLength; i++) {
        if (i >= 32) {
            let res0 = bitVector.push(1);
            assertTrue(res0, "Push should return true for bit 1");
        } else {
            let res0 = bitVector.push(0);
            assertTrue(res0, "Push should return true for bit 0");
        }
    }
    assertEQ(bitVector.length, 64, "BitVector length should be 64");
    bitVector.flipBitsByRange(0, 32);
    for (let i = 0; i < bitVectorLength; i++) {
        assertEQ(bitVector[i], 1, "All bits should be 1 after flipping");
    }
}

/**
     * @tc.number    : BITVECTOR_TEST_070
     * @tc.name      : BitVector_flipBitsByRange_Func_007
     * @tc.desc      : Test flipBitsByRange on a bit vector with initial capacity to verify correct flipping across the entire range.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
function BitVector_flipBitsByRange_Func_007() {
    let bitVector = new collections.BitVector(10);
    let bitVectorLength = 64;
    for (let i = 0; i < bitVectorLength; i++) {
        if (i >= 32) {
            let res0 = bitVector.push(1);
            assertTrue(res0, "Push should return true for bit 1");
        } else {
            let res0 = bitVector.push(0);
            assertTrue(res0, "Push should return true for bit 0");
        }
    }
    assertEQ(bitVector.length, bitVectorLength + 10, "BitVector length should be 74");
    bitVector.flipBitsByRange(10, 74);
    for (let i = 10; i < bitVectorLength + 10; i++) {
        if (i >= 42) {
            assertEQ(bitVector[i], 0, "Bits at index >= 42 should be flipped to 0");
        } else {
            assertEQ(bitVector[i], 1, "Bits at index < 42 should be flipped to 1");
        }
    }
}
