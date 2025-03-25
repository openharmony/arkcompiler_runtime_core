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
    dynamicFunctionWithUserException,
    dynamicFunctionWithCustomObject,
    dynamicFunctionWithBuiltinException
} from "dynamic_function_exception";

// 1. Dynamic function throws user-defined exception E that extends Error (catch clause with E)

// Test 1.1 Catch exception and check Error
function CheckUserExceptionError():boolean{
    try {
        dynamicFunctionWithUserException()
        return false
    } catch(e: Error) {
        return e instanceof ESError
    }

}

// Test 1.2 Check exception name
function CheckNameUserExceptionError():boolean{
    try {
        dynamicFunctionWithUserException()
        return false
    } catch(e: Error) {
         return e.name == 'CustomError1'

    }
}

// Test 1.3 Check exception message
function CheckMessageUserExceptionError():boolean{
    try {
        dynamicFunctionWithUserException()
        return false
    } catch(e: Error) {
         return e.message == 'This is a custom error'

    }
}

// 2 Dynamic function throws object that isn't extend Error

//Test 2.1 Catch exception and check Error
function CatchUserException():boolean{
    try {
        dynamicFunctionWithCustomObject()
        return false

    } catch(e: Error) {
        return e instanceof ESError && e.name ==='UnknownError' &&
               e.message == 'This is a custom object error'
    }

}


//3 Dynamic function throws builtin exception E

//Test 3.1 Catch exception and check Error
function CatchBuiltinException():boolean{
    try {
        dynamicFunctionWithBuiltinException()
        return false

    } catch(e: Error) {
        return e instanceof ESError
    }

}

// Test 3.2 Check exception name
function CheckNameBuiltinException():boolean{
    try {
        dynamicFunctionWithBuiltinException()
        return false
    } catch(e: Error) {
        return e.name ==='TypeError'
    }
}

// Test 3.3 Check exception message
function CheckMessageBuiltinException():boolean{
    try {
        dynamicFunctionWithBuiltinException()
        return false
    } catch(e: Error) {
        return e.message == 'This is a TypeError'
    }
}

