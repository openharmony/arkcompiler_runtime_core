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

const getBooleanPrimitive = etsVm.getFunction('LETSGLOBAL;', 'getBooleanPrimitive');
const getBooleanBoxed = etsVm.getFunction('LETSGLOBAL;', 'getBooleanBoxed');
const getBytePrimitive = etsVm.getFunction('LETSGLOBAL;', 'getBytePrimitive');
const getByteBoxed = etsVm.getFunction('LETSGLOBAL;', 'getByteBoxed');
const getShortPrimitive = etsVm.getFunction('LETSGLOBAL;', 'getShortPrimitive');
const getShortBoxed = etsVm.getFunction('LETSGLOBAL;', 'getShortBoxed');
const getIntPrimitive = etsVm.getFunction('LETSGLOBAL;', 'getIntPrimitive');
const getIntBoxed = etsVm.getFunction('LETSGLOBAL;', 'getIntBoxed');
const getLongPrimitive = etsVm.getFunction('LETSGLOBAL;', 'getLongPrimitive');
const getLongBoxed = etsVm.getFunction('LETSGLOBAL;', 'getLongBoxed');
const getFloatPrimitive = etsVm.getFunction('LETSGLOBAL;', 'getFloatPrimitive');
const getFloatBoxed = etsVm.getFunction('LETSGLOBAL;', 'getFloatBoxed');
const getNumberPrimitive = etsVm.getFunction('LETSGLOBAL;', 'getNumberPrimitive');
const getNumberBoxed = etsVm.getFunction('LETSGLOBAL;', 'getNumberBoxed');
const getDoublePrimitive = etsVm.getFunction('LETSGLOBAL;', 'getDoublePrimitive');
const getDoubleBoxed = etsVm.getFunction('LETSGLOBAL;', 'getDoubleBoxed');
const getStringPrimitive = etsVm.getFunction('LETSGLOBAL;', 'getStringPrimitive');
const getStringBoxed = etsVm.getFunction('LETSGLOBAL;', 'getStringBoxed');
const getCharPrimitive = etsVm.getFunction('LETSGLOBAL;', 'getCharPrimitive');
const getCharPrimitiveNumber = etsVm.getFunction('LETSGLOBAL;', 'getCharPrimitiveNumber');
const getCharBoxed = etsVm.getFunction('LETSGLOBAL;', 'getCharBoxed');
const getCharBoxedNumber = etsVm.getFunction('LETSGLOBAL;', 'getCharBoxedNumber');
const getBigIntPrimitive = etsVm.getFunction('LETSGLOBAL;', 'getBigIntPrimitive');
const getBigIntBoxed = etsVm.getFunction('LETSGLOBAL;', 'getBigIntBoxed');
const getNullPrimitive = etsVm.getFunction('LETSGLOBAL;', 'getNullPrimitive');
const getUndefinedPrimitive = etsVm.getFunction('LETSGLOBAL;', 'getUndefinedPrimitive');


function testGetBooleanPrimitive() {
    let val = getBooleanPrimitive();
    ASSERT_TRUE(typeof val === 'boolean');
    ASSERT_TRUE(val === true);
}

function testGetBooleanBoxed() {
    let val = getBooleanBoxed();
    ASSERT_TRUE(typeof val === 'boolean');
    ASSERT_TRUE(val === true);
}

function testGetBytePrimitive() {
    let val = getBytePrimitive();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetByteBoxed() {
    let val = getByteBoxed();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetShortPrimitive() {
    let val = getShortPrimitive();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetShortBoxed() {
    let val = getShortBoxed();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetIntPrimitive() {
    let val = getIntPrimitive();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetIntBoxed() {
    let val = getIntBoxed();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetLongPrimitive() {
    let val = getLongPrimitive();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetLongBoxed() {
    let val = getLongBoxed();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetFloatPrimitive() {
    let val = getFloatPrimitive();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetFloatBoxed() {
    let val = getFloatBoxed();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetNumberPrimitive() {
    let val = getNumberPrimitive();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetNumberBoxed() {
    let val = getNumberBoxed();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetDoublePrimitive() {
    let val = getDoublePrimitive();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetDoubleBoxed() {
    let val = getDoubleBoxed();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetStringPrimitive() {
    let val = getStringPrimitive();
    ASSERT_TRUE(typeof val === 'string');
    ASSERT_TRUE(val === '1');
}

function testGetStringBoxed() {
    let val = getStringBoxed();
    ASSERT_TRUE(typeof val === 'string');
    ASSERT_TRUE(val === '1');
}

function testGetCharPrimitive() {
    let val = getCharPrimitive();
    ASSERT_TRUE(typeof val === 'string');
    ASSERT_TRUE(val === '1');
}

function testGetCharPrimitiveNumber() {
    let val = getCharPrimitiveNumber();
    ASSERT_TRUE(typeof val === 'string');
    ASSERT_TRUE(val === '1');
}

function testGetCharBoxed() {
    let val = getCharBoxed();
    ASSERT_TRUE(typeof val === 'string');
    ASSERT_TRUE(val === '1');
}

function testGetCharBoxedNumber() {
    let val = getCharBoxedNumber();
    ASSERT_TRUE(typeof val === 'string');
    ASSERT_TRUE(val === '1');
}

function testGetBigIntPrimitive() {
    let val = getBigIntPrimitive();
    ASSERT_TRUE(typeof val === 'bigint');
    ASSERT_TRUE(val === 1n);
}

function testGetBigIntBoxed() {
    let val = getBigIntBoxed();
    ASSERT_TRUE(typeof val === 'bigint');
    ASSERT_TRUE(val === 1n);
}

function testGetNullPrimitive() {
    let val = getNullPrimitive();
    ASSERT_TRUE(typeof val === 'object');
    ASSERT_TRUE(val === null);
}

function testGetUndefinedPrimitive() {
    let val = getUndefinedPrimitive();
    ASSERT_TRUE(typeof val === 'undefined');
    ASSERT_TRUE(val === undefined);
}


testGetBooleanPrimitive();
testGetBooleanBoxed();
testGetBytePrimitive();
testGetByteBoxed();
testGetShortPrimitive();
testGetShortBoxed();
testGetIntPrimitive();
testGetIntBoxed();
testGetLongPrimitive();
testGetLongBoxed();
testGetIntBoxed();
testGetFloatPrimitive();
testGetFloatBoxed();
testGetNumberPrimitive();
testGetNumberBoxed();
testGetDoublePrimitive();
testGetDoubleBoxed();
testGetCharPrimitive();
testGetCharPrimitiveNumber();
testGetCharBoxed();
testGetCharBoxedNumber();
testGetStringPrimitive();
testGetStringBoxed();
testGetBigIntPrimitive();
testGetBigIntBoxed();
testGetNullPrimitive();
testGetUndefinedPrimitive();