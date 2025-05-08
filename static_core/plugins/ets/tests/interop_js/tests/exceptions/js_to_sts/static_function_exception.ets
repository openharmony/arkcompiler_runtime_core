/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
    staticFunctionWithCustomException,
    staticFunctionWithBuiltinException,
    CustomError,
    createError
} from "static_function_exception";

import {
    CustomErrorTypeA,
    createCustomErrorA
} from "ts_custom_exception";

//1 Static function throws user-defined exception E

//Test 1.1 catch and check  
function CheckCustomException():boolean{
    try {
        staticFunctionWithCustomException()
        return false
    } catch(e: Error) {
        return e instanceof ESError
    }

}

// Test 1.2 Check exception name
function CheckNameCustomException():boolean{
    try {
        staticFunctionWithCustomException()
        return false
    } catch(e: Error) {
        return e.name == 'CustomErrorA'

    }
}

// Test 1.3 Check exception message
function CheckMessageCustomException():boolean{
    try {
        staticFunctionWithCustomException()
        return false
    } catch(e: Error) {
        return e.message == 'This is a custom error(from static function)'
    }
}

// 2 Static function throws builtin exception

//Test 2.1 catch and check  
function CheckBuiltinExceptionInStatic():boolean{
    try {
        staticFunctionWithBuiltinException()
        return false
    } catch(e: Error) {
        return e instanceof TypeError
    }

}

// Test 2.2 Check exception name
function CheckNameBuiltinExceptionInStatic():boolean{
    try {
        staticFunctionWithBuiltinException()
        return false
    } catch(e: Error) {
        return e.name == 'TypeError'
    }
}

// Test 2.3 Check exception message

function CheckMessageBuiltinExceptionInStatic():boolean{
    try {
        staticFunctionWithBuiltinException()
        return false
    } catch(e: Error) {
        return e.message == 'This is a TypeError'
    }
}

// 3 Throw imported custom error

function ThrowCustomError():boolean{
    try {
        throw new CustomError('This is a custom error(from static function)') as Error
    } catch(e: Error) {
        return true
    }
}

// 4 Throw error from imported function

const myError = createError('NewError', 'New error message')

function CheckThrowNewError(): boolean {
    try {
        throw myError as Error
    } catch(e: Error) {
        return true
    }
}

// 5 Throwing an exception with an imported function and an imported type

const myErrorA: CustomErrorTypeA = createCustomErrorA('NewError', 500)

function throwTSCustomExceptionWithImportType():boolean {
    try {
        throw myErrorA as Error
    } catch(e: Error) {
        return true
    }
}

