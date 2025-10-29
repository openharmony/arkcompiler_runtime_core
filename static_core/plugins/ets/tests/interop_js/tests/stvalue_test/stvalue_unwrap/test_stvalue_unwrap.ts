/**
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

const etsVm = globalThis.gtest.etsVm;

let STValue = etsVm.STValue;
let module = STValue.findModule("stvalue_unwrap");
let stvalueWrap;
const epsilon = 1e-5;
enum SType {
    BOOLEAN,
    CHAR,
    BYTE,
    SHORT,
    INT,
    LONG,
    FLOAT,
    DOUBLE,
    REFERENCE,
    VOID
};

function testUnwrapToNumber(): void {
    stvalueWrap = STValue.wrapByte(1);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() == 1);

    stvalueWrap = STValue.wrapChar('1');
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() == 49); // ASCII code for '1'

    stvalueWrap = STValue.wrapShort(32767);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() == 32767);

    stvalueWrap = STValue.wrapShort(-32768);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() == -32768);

    stvalueWrap = STValue.wrapInt(44);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() == 44);

    stvalueWrap = STValue.wrapLong(44);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() == 44);


    stvalueWrap = STValue.wrapFloat(44.4);
    ASSERT_TRUE(Math.abs(stvalueWrap.unwrapToNumber() - 44.4) <= epsilon);

    stvalueWrap = STValue.wrapNumber(44.4);
    ASSERT_TRUE(Math.abs(stvalueWrap.unwrapToNumber() - 44.4) <= epsilon);

    stvalueWrap = STValue.wrapBoolean(true);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() == true);
}

function testUnwrapToString(): void {
    // successfully trigger the exception: cannot unwrap a primitive value to string
    // let magicSTValueInt = module.moduleGetVariable('magicInt', SType.INT); 
    // let magicInt = magicSTValueInt.unwrapToString();
    // print('magicInt: ', magicInt);

    let magicSTValue1 = module.moduleGetVariable('magicString1', SType.REFERENCE);
    let magicString1 = magicSTValue1.unwrapToString();
    print('magicString1: ', magicString1);
    ASSERT_TRUE(magicString1 === 'Hello World');

    let magicSTValue2 = module.moduleGetVariable('magicString2', SType.REFERENCE);
    let magicString2 = magicSTValue2.unwrapToString();
    print('magicString2: ', magicString2);
    ASSERT_TRUE(magicString2 === 'Hello World!!!');
}

function testUnwrapToBigInt(): void {
    // testUnwrapToBigInt
    const bigIntFromStringValue = module.moduleGetVariable('bigIntFromString', SType.REFERENCE);
    const bigIntFromString = bigIntFromStringValue.unwrapToBigInt();

    ASSERT_TRUE(bigIntFromString === 999999999999999999n);
    ASSERT_TRUE(typeof bigIntFromString === 'bigint');

    const bigIntFromLiteralValue = module.moduleGetVariable('bigIntFromLiteral', SType.REFERENCE);
    const bigIntFromLiteral = bigIntFromLiteralValue.unwrapToBigInt();

    ASSERT_TRUE(bigIntFromLiteral === 123456789012345678901234567890n);
    ASSERT_TRUE(typeof bigIntFromLiteral === 'bigint');

    // testArithmeticOperations
    const bigIntValue = module.moduleGetVariable('bigIntFromString', SType.REFERENCE).unwrapToBigInt();

    const additionResult = bigIntValue + 1n;
    const multiplicationResult = bigIntValue * 2n;
    const subtractionResult = bigIntValue - 1000n;

    ASSERT_TRUE(additionResult === 1000000000000000000n);
    ASSERT_TRUE(multiplicationResult === 1999999999999999998n);
    ASSERT_TRUE(subtractionResult === 999999999999998999n);

    // testTypeCheck
    try {
        const numberValue = module.moduleGetVariable('defaultNumber', SType.REFERENCE);
        const result = numberValue.unwrapToBigInt();
        ASSERT_TRUE(false, 'Should have thrown error for non-BigInt conversion');
    } catch (error) {
        print('Expected error for non-BigInt conversion:', error.message);
        ASSERT_TRUE(true, 'Correctly threw error for non-BigInt conversion');
    }

    try {
        const stringVariable = module.moduleGetVariable('stringVariable', SType.REFERENCE);
        const result = stringVariable.unwrapToBigInt();
        print('Non-BigInt conversion result:', result);
        ASSERT_TRUE(false, 'Should have thrown error for non-BigInt conversion');
    } catch (error) {
        print('Expected error for non-BigInt conversion:', error.message);
        ASSERT_TRUE(true, 'Correctly threw error for non-BigInt conversion');
    }

    // testEdgeCases
    let zeroSTValue = module.moduleGetVariable('zeroBigInt', SType.REFERENCE);
    let zeroBigInt = zeroSTValue.unwrapToBigInt();

    ASSERT_TRUE(zeroBigInt === 0n);

    let negativeSTValue = module.moduleGetVariable('negativeBigInt', SType.REFERENCE);
    let negativeBigInt = negativeSTValue.unwrapToBigInt();

    ASSERT_TRUE(negativeBigInt === -1234567890123456789n);
}
function testUnwrapToBoolean(): void {
    stvalueWrap = STValue.wrapByte(127);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() == true);

    stvalueWrap = STValue.wrapByte(0);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() == false);

    stvalueWrap = STValue.wrapByte(-44);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() == true);

    stvalueWrap = STValue.wrapByte(-128);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() == true); // -128 = 0b(10000000) ?

    stvalueWrap = STValue.wrapInt(44);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() == true);

    stvalueWrap = STValue.wrapInt(0);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() == false);

    stvalueWrap = STValue.wrapInt(-44);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() == true);

    stvalueWrap = STValue.wrapInt(-2147483648);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() == false);

    stvalueWrap = STValue.wrapShort(32767);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() == true);

    stvalueWrap = STValue.wrapShort(-32767);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() == true);

    stvalueWrap = STValue.wrapShort(-32768);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() == false); //-32768 = 0b(10000000 00000000)

    stvalueWrap = STValue.wrapLong(44);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() == true);

    stvalueWrap = STValue.wrapLong(-44);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() == true);

    stvalueWrap = STValue.wrapChar('1');
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() == true); // ASCII code for '1'

    stvalueWrap = STValue.wrapChar('0');
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() == true); // ASCII code for '0'

    stvalueWrap = STValue.wrapFloat(44.4);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() == true);

    stvalueWrap = STValue.wrapFloat(0);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() == false);

    stvalueWrap = STValue.wrapNumber(44.4);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() == true);

    stvalueWrap = STValue.wrapNumber(0);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() == false);
    print('UnwrapToBoolean Test SUCCESS!')
}


function main(): void {
    testUnwrapToNumber();
    testUnwrapToString();
    testUnwrapToBigInt();
    testUnwrapToBoolean();
}

main();
