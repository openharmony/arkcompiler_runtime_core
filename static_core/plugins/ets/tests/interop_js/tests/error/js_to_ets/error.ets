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

let module = ESValue.load('../../plugins/ets/tests/interop_js/tests/error/js_to_ets/error')
let throwNum = module.getProperty('throwNum');
let throwStr = module.getProperty('throwStr');
let throwObj = module.getProperty('throwObj');
let throwErrorSubClass = module.getProperty('throwErrorSubClass');
let throwError1 = module.getProperty('throwError1');
let throwError2 = module.getProperty('throwError2');
let throwError3 = module.getProperty('throwError3');
let throwEtsError = module.getProperty('throwEtsError');

function throwError(): Error {
    throw new Error('Error From ETS');
}

function checkEtsError(): boolean {
    try { throwEtsError.invoke(throwError); } catch (e: Error) {
        return e.message === 'Error From ETS';
    }
    return false;
}

function checkNumError(): boolean {
    try { throwNum.invoke(); } catch (e: Error) {
        let errObj: ESValue = (e as ESError).getValue();
        return errObj.toNumber() === 0;
    }
    return false;
}

function checkStrError(): boolean {
    try { throwStr.invoke(); } catch (e: Error) {
        let errObj: ESValue = (e as ESError).getValue();
        return errObj.toString() === 'hello';
    }
    return false;
}

function checkObjError(): boolean {
    try { throwObj.invoke(); } catch (e: Error) {
        let errObj: ESValue = (e as ESError).getValue();
        let a: ESValue = errObj.getProperty('a');
        let b: ESValue = errObj.getProperty('b');
        return (a.toNumber() === 1) && (b.toNumber() === 2);
    }
    return false;
}

function checkSubClassError(): boolean {
    try { throwErrorSubClass.invoke(); } catch (e: Error) {
        let errObj: ESValue = (e as ESError).getValue();
        let extraField: ESValue = errObj.getProperty('extraField');
        let foo: ESValue = errObj.getProperty('foo');
        return (extraField.toNumber() === 123) && (foo.invoke().toNumber() === 456);
    }
    return false;
}

function checkErrorData(): boolean {
    try { throwError1.invoke(); } catch (e: Error) {
        let errObj: ESValue = (e as ESError).getValue();
        let msg: ESValue = errObj.getProperty('message');
        let data: ESValue = msg.getProperty('data');
        return data.toNumber() === 123;
    }
    return false;
}

function checkErrorMsg(): boolean {
    try { throwError2.invoke(); } catch (e: Error) {
        return e.message === 'null pointer error';
    }
    return false;
}

function checkErrorCause(): boolean {
    try { throwError3.invoke(); } catch (e: Error) {
        let errObj: ESValue = (e as ESError).getValue();
        let curMsg: ESValue = errObj.getProperty('message');
        let cause: ESValue = errObj.getProperty('cause');
        let orgMsg: ESValue = cause.getProperty('message');
        return curMsg.toString() === 'current error' && orgMsg.toString() === 'original error';
    }
    return false;
}

// Check Error
function checkError(): boolean {
    let cause: ErrorOptions = new ErrorOptions('Test cause');
    let err = new Error('original error', cause);
    let orgName = err.name;
    let orgMsg = err.message;
    let orgCause = err.cause;
    err.name = 'new error name';
    err.message = 'new error message';
    err.cause = 'new error cause';

    return orgName === 'Error' && orgMsg === 'original error' &&
        orgCause === 'Test cause' && err.cause === 'new error cause' &&
        err.name === 'new error name' && err.message === 'new error message';
}