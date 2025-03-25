
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

package exceptions.test;

function throwErrorFromSts(): void {
    throw new Error("Throw Error from ets!");
}

function throwErrorWithCauseFromSts(): void {
    let cause: ErrorOptions = new ErrorOptions("Test cause");
    throw new Error("Throw Error with cause from ets!", cause);
}

class CustomError extends Error {
    code: number;
    constructor(message: string, code: number) {
        super(message);
        this.name = "CustomError";
        this.code = code;
    }
}

function throwCustomErrorFromSts(): void {
    throw new CustomError("Throw Custom Error from ets!", 12345);
}

function throwExceptionFromSts(): void throws {
    throw new Exception("Throw Exception from ets!");
}

class ExceptionCause {
    reason: String = "";
}

function throwExceptionWithCauseFromSts(): void throws {
    let cause: ExceptionCause = { reason: "Test cause" };
    throw new Exception("Throw Exception with cause from ets!", cause);
}

class CustomException extends Exception {
    code: number;
    constructor(message: string, code: number) {
        super(message);
        this.code = code;
    }
}

function throwCustomExceptionFromSts(): void throws {
    throw new CustomException("Throw Custom Exception from ets!", 54321);
}
