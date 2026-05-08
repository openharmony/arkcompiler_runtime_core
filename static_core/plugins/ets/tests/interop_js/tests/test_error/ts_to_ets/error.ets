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

const module = ESValue.load('test_error');
const throwRangeError = module.getProperty('throwRangeError');
const throwRangeErrorWithMessage = module.getProperty('throwRangeErrorWithMessage');
const throwReferenceError = module.getProperty('throwReferenceError');
const throwReferenceErrorWithMessage = module.getProperty('throwReferenceErrorWithMessage');
const throwSyntaxError = module.getProperty('throwSyntaxError');
const throwSyntaxErrorWithMessage = module.getProperty('throwSyntaxErrorWithMessage');
const throwURIError = module.getProperty('throwURIError');
const throwURIErrorWithMessage = module.getProperty('throwURIErrorWithMessage');
const throwTypeError = module.getProperty('throwTypeError');
const throwTypeErrorWithMessage = module.getProperty('throwTypeErrorWithMessage');
const throwEvalError = module.getProperty('throwEvalError');
const throwEvalErrorWithMessage = module.getProperty('throwEvalErrorWithMessage');
const throwAggregateError = module.getProperty('throwAggregateError');
const throwAggregateErrorWithMessage = module.getProperty('throwAggregateErrorWithMessage');
const testExtendThrowRangeError = module.getProperty('testExtendThrowRangeError');
const testExtendThrowReferenceError = module.getProperty('testExtendThrowReferenceError');
const testExtendThrowSyntaxError = module.getProperty('testExtendThrowSyntaxError');
const testExtendThrowURIError = module.getProperty('testExtendThrowURIError');
const testExtendThrowTypeError = module.getProperty('testExtendThrowTypeError');
const throwNum = module.getProperty('throwNum');
const throwStr = module.getProperty('throwStr');
const throwObj = module.getProperty('throwObj');
const throwErrorSubClass = module.getProperty('throwErrorSubClass');
const throwError1 = module.getProperty('throwError1');
const throwError2 = module.getProperty('throwError2');
const throwError3 = module.getProperty('throwError3');

const MESSAGE: string = 'Test Error Message';

function testCatchRangeError(): boolean {
    let res: boolean;
    try {
        throwRangeError.invoke();
        res = false;
    } catch (e) {
        res = (e instanceof RangeError) && (e.name === 'RangeError');
    }
    try {
        throwRangeErrorWithMessage.invoke(MESSAGE);
        res = res && false;
    } catch (e) {
        res = res && (e instanceof RangeError) && (e.name === 'RangeError') && (e.message === MESSAGE);
    }
    return res;
}

function testCatchReferenceError(): boolean {
    let res: boolean;
    try {
        throwReferenceError.invoke();
        res = false;
    } catch (e) {
        res = (e instanceof ReferenceError) && (e.name === 'ReferenceError');
    }
    try {
        throwReferenceErrorWithMessage.invoke(MESSAGE);
        res = res && false;
    } catch (e) {
        res = res && (e instanceof ReferenceError) && (e.name === 'ReferenceError') && (e.message === MESSAGE);
    }
    return res;
}

function testCatchSyntaxError(): boolean {
    let res: boolean;
    try {
        throwSyntaxError.invoke();
        res = false;
    } catch (e) {
        res = (e instanceof SyntaxError) && (e.name === 'SyntaxError');
    }
    try {
        throwSyntaxErrorWithMessage.invoke(MESSAGE);
        res = res && false;
    } catch (e) {
        res = res && (e instanceof SyntaxError) && (e.name === 'SyntaxError') && (e.message === MESSAGE);
    }
    return res;
}

function testCatchURIError(): boolean {
    let res: boolean;
    try {
        throwURIError.invoke();
        res = false;
    } catch (e) {
        res = (e instanceof URIError) && (e.name === 'URIError');
    }
    try {
        throwURIErrorWithMessage.invoke(MESSAGE);
        res = res && false;
    } catch (e) {
        res = res && (e instanceof URIError) && (e.name === 'URIError') && (e.message === MESSAGE);
    }
    return res;
}

function testCatchTypeError(): boolean {
    let res: boolean;
    try {
        throwTypeError.invoke();
        res = false;
    } catch (e) {
        res = (e instanceof TypeError) && (e.name === 'TypeError');
    }
    try {
        throwTypeErrorWithMessage.invoke(MESSAGE);
        res = res && false;
    } catch (e) {
        res = res && (e instanceof TypeError) && (e.name === 'TypeError') && (e.message === MESSAGE);
    }
    return res;
}

function testCatchEvalError(): boolean {
    let res: boolean;
    try {
        throwEvalError.invoke();
        res = false;
    } catch (e) {
        res = (e instanceof Error) && (e.name === 'EvalError');
    }
    try {
        throwEvalErrorWithMessage.invoke(MESSAGE);
        res = res && false;
    } catch (e) {
        res = res && (e instanceof Error) && (e.name === 'EvalError') && (e.message === MESSAGE);
    }
    return res;
}

function testCatchAggregateError(): boolean {
    let res: boolean;
    try {
        throwAggregateError.invoke();
        res = false;
    } catch (e) {
        res = (e instanceof Error) && (e.name === 'AggregateError');
    }
    try {
        throwAggregateErrorWithMessage.invoke(MESSAGE);
        res = res && false;
    } catch (e) {
        res = res && (e instanceof Error) && (e.name === 'AggregateError') && (e.message === MESSAGE);
    }
    return res;
}

function testCatchExtendRangeError(): boolean {
    let res: boolean;
    try {
        testExtendThrowRangeError.invoke(MESSAGE);
        res = false;
    } catch (e) {
        res = (e instanceof RangeError) && (e.name === 'ExtendRangeError') && (e.message === MESSAGE);
    }
    return res;
}

function testCatchExtendReferenceError(): boolean {
    let res: boolean;
    try {
        testExtendThrowReferenceError.invoke(MESSAGE);
        res = false;
    } catch (e) {
        res = (e instanceof ReferenceError) && (e.name === 'ExtendReferenceError') && (e.message === MESSAGE);
    }
    return res;
}

function testCatchExtendSyntaxError(): boolean {
    let res: boolean;
    try {
        testExtendThrowSyntaxError.invoke(MESSAGE);
        res = false;
    } catch (e) {
        res = (e instanceof SyntaxError) && (e.name === 'ExtendSyntaxError') && (e.message === MESSAGE);
    }
    return res;
}

function testCatchExtendURIError(): boolean {
    let res: boolean;
    try {
        testExtendThrowURIError.invoke(MESSAGE);
        res = false;
    } catch (e) {
        res = (e instanceof URIError) && (e.name === 'ExtendURIError') && (e.message === MESSAGE);
    }
    return res;
}

function testCatchExtendTypeError(): boolean {
    let res: boolean;
    try {
        testExtendThrowTypeError.invoke(MESSAGE);
        res = false;
    } catch (e) {
        res = (e instanceof TypeError) && (e.name === 'ExtendTypeError') && (e.message === MESSAGE);
    }
    return res;
}

function testCatchNonError(): boolean {
    let res: boolean;
    try {
        throwNum.invoke();
        res = false;
    } catch (e) {
        let errObj: ESValue = (e as ESError).getValue();
        res = errObj.toNumber() === 0;
    }
    try {
        throwStr.invoke();
        res = res && false;
    } catch (e) {
        let errObj1: ESValue = (e as ESError).getValue();
        res = res && errObj1.toString() === 'hello';
    }
    try {
        throwObj.invoke();
        res = res && false;
    } catch (e) {
        let errObj2: ESValue = (e as ESError).getValue();
        let a: ESValue = errObj2.getProperty('a');
        let b: ESValue = errObj2.getProperty('b');
        res = res && (a.toNumber() === 1) && (b.toNumber() === 2);
    }
    return res;
}

function testCatchErrorSubClass(): boolean {
    let res: boolean;
    try {
        throwErrorSubClass.invoke();
        res = false;
    } catch (e) {
        let err = ESValue.wrap(e);
        res = (e instanceof Error) &&
            (err.getProperty('extraField').toNumber() === 123) &&
            (err.invokeMethod('foo').toNumber() === 456);
    }
    return res;
}

function testCatchErrorWithObjectMessage(): boolean {
    let res: boolean;
    try {
        throwError1.invoke();
        res = false;
    } catch (e) {
        res = (e instanceof Error);
    }
    return res;
}

function testCatchErrorWithMessage(): boolean {
    let res: boolean;
    try {
        throwError2.invoke();
        res = false;
    } catch (e) {
        res = (e instanceof Error) && (e.message === 'null pointer error');
    }
    return res;
}

function testCatchErrorWithCause(): boolean {
    let res: boolean;
    try {
        throwError3.invoke();
        res = false;
    } catch (e) {
        res = (e instanceof Error) && (e.message === 'current error') && (e.cause instanceof Error);
    }
    return res;
}