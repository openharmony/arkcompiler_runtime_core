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

import {
    err,
    ErrorFunc,
    CustomErrorFunc
} from 'test_error_ts';

const ErrMsg: string = 'This is a Error';
const ErrorMessage: string = 'Panda: throw new error in ets!';
const CustomErrorMessage: string = 'Panda: Something went wrong!';
const CustomErrorCode: number = 1001;

function testError(): boolean {
    let res: boolean;
    res = (err.message == ErrMsg);
    return res;
}

function testThrowError(): boolean {
    let res: boolean;
    try {
        ErrorFunc(ErrorMessage);
        res = false;
    } catch (error: Error) {
        res = (error instanceof Error) && (error.message == ErrorMessage);
    }
    return res;
}

function testCustomThrowError(): boolean {
    let res: boolean;
    try {
        CustomErrorFunc(CustomErrorMessage, CustomErrorCode);
        res = false;
    } catch (e: Error) {
        console.log(e instanceof Error);
        let errObj: ESObject = (e as ESError).getValue()
        let msg: ESObject = errObj.getProperty('message')
        let code: ESObject = errObj.getProperty('code')
        res = (e instanceof Error) && (msg.toString() == CustomErrorMessage) &&
              (code.toNumber() == CustomErrorCode);
    }
    return res;
}