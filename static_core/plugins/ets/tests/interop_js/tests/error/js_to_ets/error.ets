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

import {
    throwNum,
    throwStr,
    throwObj,
    throwErrorSubClass,
    throwError1,
    throwError2,
    throwError3,
    throwEtsError
} from "error_js"

function throwError() {
    throw new Error('Error From ETS')
}

function checkEtsError(): boolean {
    try { throwEtsError(throwError) } catch (e: Error) {
        return e.message == 'Error From ETS'
    }
    return false
}

function checkNumError(): boolean {
    try { throwNum() } catch (e: Error) {
        let errObj: ESObject = (e as ESError).getValue()
        return errObj.toNumber() == 0
    }
    return false
}

function checkStrError(): boolean {
    try { throwStr() } catch (e: Error) {
        let errObj: ESObject = (e as ESError).getValue()
        return errObj.toString() == 'hello'
    }
    return false
}

function checkObjError(): boolean {
    try { throwObj() } catch (e: Error) {
        let errObj: ESObject = (e as ESError).getValue()
        let a: ESObject = errObj.getProperty('a')
        let b: ESObject = errObj.getProperty('b')
        return (a.toNumber() == 1) && (b.toNumber() == 2)
    }
    return false
}

function checkSubClassError(): boolean {
    try { throwErrorSubClass() } catch (e: Error) {
        let errObj: ESObject = (e as ESError).getValue()
        let extraField: ESObject = errObj.getProperty('extraField')
        let foo: ESObject = errObj.getProperty('foo')
        return (extraField.toNumber() == 123) && (foo.invoke().toNumber() == 456)
    }
    return false
}

function checkErrorData(): boolean {
    try { throwError1() } catch (e: Error) {
        let errObj: ESObject = (e as ESError).getValue()
        let msg: ESObject = errObj.getProperty('message')
        let data: ESObject = msg.getProperty('data')
        return data.toNumber() == 123
    }
    return false
}

function checkErrorMsg(): boolean {
    try { throwError2() } catch (e: Error) {
        return e.message == "null pointer error"
    }
    return false
}

function checkErrorCause(): boolean {
    try { throwError3() } catch (e: Error) {
        let errObj: ESObject = (e as ESError).getValue()
        let cur_msg: ESObject = errObj.getProperty('message')
        let cause: ESObject = errObj.getProperty('cause')
        let org_msg: ESObject = cause.getProperty('message')
        return cur_msg.toString() == "current error" && org_msg.toString() == "original error"
    }
    return false
}

// Check Error

function checkError(): boolean {
    let cause: ErrorOptions = new ErrorOptions("Test cause");
    let err = new Error("original error", cause)
    let org_name = err.name
    let org_msg = err.message
    let org_cause = err.cause
    err.name = "new error name"
    err.message = "new error message"
    err.cause = "new error cause"

    return org_name == "Error" && org_msg == "original error" &&
           org_cause == "Test cause" && err.cause == "new error cause" &&
           err.name == "new error name" && err.message == "new error message"
}