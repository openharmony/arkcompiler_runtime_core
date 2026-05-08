/**
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

const etsVm = globalThis.gtest.etsVm;

const rangeErr = etsVm.getClass('Ltest_error/ETSGLOBAL;').rangeErr;
const referenceErr = etsVm.getClass('Ltest_error/ETSGLOBAL;').referenceErr;
const syntaxErr = etsVm.getClass('Ltest_error/ETSGLOBAL;').syntaxErr;
const uriErr = etsVm.getClass('Ltest_error/ETSGLOBAL;').uriErr;
const typeErr = etsVm.getClass('Ltest_error/ETSGLOBAL;').typeErr;
const linkerClassNotFoundError = etsVm.getClass('Ltest_error/ETSGLOBAL;').linkerClassNotFoundError;
const extendRangeError = etsVm.getClass('Ltest_error/ETSGLOBAL;').extendRangeError;
const extendReferenceError = etsVm.getClass('Ltest_error/ETSGLOBAL;').extendReferenceError;
const extendSyntaxError = etsVm.getClass('Ltest_error/ETSGLOBAL;').extendSyntaxError;
const extendURIError = etsVm.getClass('Ltest_error/ETSGLOBAL;').extendURIError;
const extendTypeError = etsVm.getClass('Ltest_error/ETSGLOBAL;').extendTypeError;

let testThrowRangeError = etsVm.getFunction('Ltest_error/ETSGLOBAL;', 'testThrowRangeError');
let testThrowReferenceError = etsVm.getFunction('Ltest_error/ETSGLOBAL;', 'testThrowReferenceError');
let testThrowSyntaxError = etsVm.getFunction('Ltest_error/ETSGLOBAL;', 'testThrowSyntaxError');
let testThrowURIError = etsVm.getFunction('Ltest_error/ETSGLOBAL;', 'testThrowURIError');
let testThrowTypeError = etsVm.getFunction('Ltest_error/ETSGLOBAL;', 'testThrowTypeError');
let testThrowLinkerClassNotFoundError = etsVm.getFunction('Ltest_error/ETSGLOBAL;', 'testThrowLinkerClassNotFoundError');

let testRangeError = etsVm.getFunction('Ltest_error/ETSGLOBAL;', 'testRangeError');
let testReferenceError = etsVm.getFunction('Ltest_error/ETSGLOBAL;', 'testReferenceError');
let testSyntaxError = etsVm.getFunction('Ltest_error/ETSGLOBAL;', 'testSyntaxError');
let testURIError = etsVm.getFunction('Ltest_error/ETSGLOBAL;', 'testURIError');
let testTypeError = etsVm.getFunction('Ltest_error/ETSGLOBAL;', 'testTypeError');

let testExtendThrowRangeError = etsVm.getFunction('Ltest_error/ETSGLOBAL;', 'testExtendThrowRangeError');
let testExtendThrowReferenceError = etsVm.getFunction('Ltest_error/ETSGLOBAL;', 'testExtendThrowReferenceError');
let testExtendThrowSyntaxError = etsVm.getFunction('Ltest_error/ETSGLOBAL;', 'testExtendThrowSyntaxError');
let testExtendThrowURIError = etsVm.getFunction('Ltest_error/ETSGLOBAL;', 'testExtendThrowURIError');
let testExtendThrowTypeError = etsVm.getFunction('Ltest_error/ETSGLOBAL;', 'testExtendThrowTypeError');

function testCheckErrorName(): boolean {
    let res: boolean;
    let ranError: Object = testRangeError('Error Message');
    let refError: Object = testReferenceError('Error Message');
    let synError: Object = testSyntaxError('Error Message');
    let uriError: Object = testURIError('Error Message');
    let typeError: Object = testTypeError('Error Message');
    res = (rangeErr.name === 'RangeError') &&
        (referenceErr.name === 'ReferenceError') &&
        (syntaxErr.name === 'SyntaxError') &&
        (uriErr.name === 'URIError') &&
        (typeErr.name === 'TypeError') &&
        (ranError.name === 'RangeError') &&
        (refError.name === 'ReferenceError') &&
        (synError.name === 'SyntaxError') &&
        (uriError.name === 'URIError') &&
        (typeError.name === 'TypeError') &&
        (linkerClassNotFoundError.name === 'LinkerClassNotFoundError') &&
        (extendRangeError.name === 'ExtendRangeError') &&
        (extendReferenceError.name === 'ExtendReferenceError') &&
        (extendSyntaxError.name === 'ExtendSyntaxError') &&
        (extendURIError.name === 'ExtendURIError') &&
        (extendTypeError.name === 'ExtendTypeError') &&
        (rangeErr instanceof RangeError) &&
        (referenceErr instanceof ReferenceError) &&
        (syntaxErr instanceof SyntaxError) &&
        (uriErr instanceof URIError) &&
        (typeErr instanceof TypeError) &&
        (ranError instanceof RangeError) &&
        (refError instanceof ReferenceError) &&
        (synError instanceof SyntaxError) &&
        (uriError instanceof URIError) &&
        (typeError instanceof TypeError) &&
        (extendRangeError instanceof RangeError) &&
        (extendReferenceError instanceof ReferenceError) &&
        (extendSyntaxError instanceof SyntaxError) &&
        (extendURIError instanceof URIError) &&
        (extendTypeError instanceof TypeError);
    return res;
}

function testCheckCatchError(): boolean {
    let res: boolean = true;
    try {
        testThrowRangeError();
        res = false;
    } catch (e) {
        res = res && (e.name === 'RangeError') && (e instanceof RangeError);
    }
    try {
        testThrowReferenceError();
        res = false;
    } catch (e) {
        res = res && (e.name === 'ReferenceError') && (e instanceof ReferenceError);
    }
    try {
        testThrowSyntaxError();
        res = false;
    } catch (e) {
        res = res && (e.name === 'SyntaxError') && (e instanceof SyntaxError);
    }
    try {
        testThrowURIError();
        res = false;
    } catch (e) {
        res = res && (e.name === 'URIError') && (e instanceof URIError);
    }
    try {
        testThrowTypeError();
        res = false;
    } catch (e) {
        res = res && (e.name === 'TypeError') && (e instanceof TypeError);
    }
    try {
        testThrowLinkerClassNotFoundError();
    } catch (e) {
        res = res && (e instanceof Error) && (e.name === 'LinkerClassNotFoundError');
    }
    return res;
}

function testCheckExtendError(): boolean {
    let res: boolean = true;
    try {
        testExtendThrowRangeError('extend Error!');
        res = false;
    } catch (err) {
        res = res && (err instanceof RangeError) && (err.name === 'ExtendRangeError') &&
            (err.message === 'extend Error!');
    }
    try {
        testExtendThrowReferenceError('extend Error!');
        res = false;
    } catch (err) {
        res = res && (err instanceof ReferenceError) && (err.name === 'ExtendReferenceError') &&
            (err.message === 'extend Error!');
    }
    try {
        testExtendThrowSyntaxError('extend Error!');
        res = false;
    } catch (err) {
        res = res && (err instanceof SyntaxError) && (err.name === 'ExtendSyntaxError') &&
            (err.message === 'extend Error!');
    }
    try {
        testExtendThrowURIError('extend Error!');
        res = false;
    } catch (err) {
        res = res && (err instanceof URIError) && (err.name === 'ExtendURIError') &&
            (err.message === 'extend Error!');
    }
    try {
        testExtendThrowTypeError('extend Error!');
        res = false;
    } catch (err) {
        res = res && (err instanceof TypeError) && (err.name === 'ExtendTypeError') &&
            (err.message === 'extend Error!');
    }
    return res;
}

ASSERT_TRUE(testCheckErrorName());
ASSERT_TRUE(testCheckCatchError());
ASSERT_TRUE(testCheckExtendError());