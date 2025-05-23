/*
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

const success = 0;
const fail = 1;

class ExceptionCause {
    reason: String = "";
}

function main(): int {
    let failures = 0;

    failures += test(testThrowException(), "Throw an Exception and catch it");
    failures += test(testExceptionWithCause(), "Exception object with cause");
    failures += test(testThrowRuntimeException(), "Throw an RuntimeException object with message & cause");
    failures += test(testThrowRuntimeExceptionWithoutArgs(), "Throw an RuntimeException object");
    failures += test(testThrowRuntimeExceptionWithoutReason(), "Throw an RuntimeException object with message");
    return failures;
}

function testThrowException(): int {
    try {
        throw new Exception("Test throw");
    } catch (ex) {
        if (ex instanceof Exception) {
            return success;
        }
        return fail;
    }
}

function testExceptionWithCause(): int {
    try {
        let cause: ExceptionCause = { reason: "Test cause" };
        throw new Exception("Test Exception with cause", cause);
    } catch (ex) {
        if (ex instanceof Exception) {
            return success;
        }
        return fail;
    }
}

function testThrowRuntimeException(): int {
    try {
        let cause: ExceptionCause = { reason: "Test cause" };
        throw new RuntimeException("Test RuntimeException", cause);
    }
    catch (ex) {
        if (ex instanceof RuntimeException) {
            return success;
        }
        return fail;
    }
}

function testThrowRuntimeExceptionWithoutArgs(): int {
    try {
        throw new RuntimeException();
    }
    catch (ex) {
        if (ex instanceof RuntimeException) {
            return success;
        }
        return fail;
    }
}

function testThrowRuntimeExceptionWithoutReason(): int {
    try {
        throw new RuntimeException("Test RuntimeException");
    }
    catch (ex) {
        if (ex instanceof RuntimeException) {
            return success;
        }
        return fail;
    }
}


function test(result: int, message: String): int {
    if (result == 0) {
      return success;
    }
    console.println("FAILED: " + message);
    return fail;
}
