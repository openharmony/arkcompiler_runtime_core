/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

class ErrorCause {
    reason: String;
}

function main(): int {
    let failures = 0;

    failures += test(testCreateError(), "Create an Error object with a message");
    failures += test(testThrowError(), "Throw an Error and catch it");
    failures += test(testErrorMessage(), "Validate the message of the caught Error");
    failures += test(testErrorWithCause(), "Error object with cause");
    failures += test(testCreateAggregateError(), "Create an AggregateError object with an array of errors");
    failures += test(testAggregateErrorMessage(), "Validate the message of the AggregateError");
    failures += test(testEvalError(), "EvalError with message and options");
    failures += test(testRangeError(), "RangeError with message and options");
    failures += test(testReferenceError(), "ReferenceError with message and options");
    failures += test(testSyntaxError(), "SyntaxError with message and options");
    failures += test(testTypeError(), "TypeError with message and options");
    failures += test(testURIError(), "URIError with message and options");
    return test(failures, "All tests run");
}

function testCreateError(): int {
    let err = new Error("Test error");
    if (err.message == "Test error") return 0;
    return 1;
}

function testThrowError(): int {
    try {
        throw new Error("Test throw");
    } catch (err: Error) {
        return 0;
    }
}

function testErrorMessage(): int {
    try {
        throw new Error("Error message test");
    } catch (err: Error) {
        if (err.message == "Error message test") return 0;
        return 1;
    }
}

function testErrorWithCause(): int {
    let cause: ErrorCause = { reason: "Test cause" };
    let err = new Error("Test error with cause", new ErrorOptions(cause));
    if (err.cause == cause) return 0;
    return 1;
}

function testCreateAggregateError(): int {
    let errors = [new Error("Error 1"), new Error("Error 2")];
    let aggErr = new AggregateError(errors, "Aggregate error message");
    if (aggErr.errors.length == 2 && aggErr.message == "Aggregate error message") return 0;
    return 1;
}

function testAggregateErrorMessage(): int {
    let aggErr = new AggregateError([] as Error[], "Test aggregate error message");
    if (aggErr.message == "Test aggregate error message") return 0;
    return 1;
}

function testEvalError(): int {
    let err = new EvalError("Test EvalError", new ErrorOptions("Eval cause"));
    if (err.message == "Test EvalError" && err.name == "EvalError") return 0;
    return 1;
}

function testRangeError(): int {
    let err = new RangeError("Test RangeError", new ErrorOptions("Range cause"));
    if (err.message == "Test RangeError" && err.name == "RangeError") return 0;
    return 1;
}

function testReferenceError(): int {
    let err = new ReferenceError("Test ReferenceError", new ErrorOptions("Reference cause"));
    if (err.message == "Test ReferenceError" && err.name == "ReferenceError") return 0;
    return 1;
}

function testSyntaxError(): int {
    let err = new SyntaxError("Test SyntaxError", new ErrorOptions("Syntax cause"));
    if (err.message == "Test SyntaxError" && err.name == "SyntaxError") return 0;
    return 1;
}

function testTypeError(): int {
    let err = new TypeError("Test TypeError", new ErrorOptions("Type cause"));
    if (err.message == "Test TypeError" && err.name == "TypeError") return 0;
    return 1;
}

function testURIError(): int {
    let err = new URIError("Test URIError", new ErrorOptions("URI cause"));
    if (err.message == "Test URIError" && err.name == "URIError") return 0;
    return 1;
}

function test(result: int, message: String): int {
    if (result == 0) {
      return 0;
    }
    console.println("FAILED: " + message);
    return 1;
}
