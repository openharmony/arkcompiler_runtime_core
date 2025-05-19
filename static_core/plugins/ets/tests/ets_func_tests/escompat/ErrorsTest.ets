/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

const success = 0
const fail = 1

function main(): int {
    const suite = new ArkTestsuite("Errors API tests")

    suite.addTest("Create an Error object with a message", testCreateError)
    suite.addTest("Throw an Error and catch it", testThrowError)
    suite.addTest("Validate the message of the caught Error", testErrorMessage)
    suite.addTest("Error object with cause", testErrorWithCause)
    suite.addTest("Create an AggregateError object with an array of errors", testCreateAggregateError)
    suite.addTest("Create an AggregateError object with an Array<Error>", testCreateAggregateErrorFromArray)
    suite.addTest("Validate the message of the AggregateError", testAggregateErrorMessage)
    suite.addTest("EvalError with message and options", testEvalError)
    suite.addTest("RangeError with message and options", testRangeError)
    suite.addTest("ReferenceError with message and options", testReferenceError)
    suite.addTest("SyntaxError with message and options", testSyntaxError)
    suite.addTest("TypeError with message and options", testTypeError)
    suite.addTest("URIError with message and options", testURIError)

    return suite.run()
}

function checkMessageAndName(
    err: Error,
    errSc: Error,
    expected_message: string,
    expected_name: string
): boolean {
    const result =
        errSc.message == err.message &&
        err.message == expected_message &&
        errSc.name == err.name &&
        err.name == expected_name
    return result
}

function testErrorHelper(
    factoryUsingConstructor: (x?: string) => Error,
    factoryUsingStaticInvoke: (x?: string) => Error,
    name: string,
    message: string
) {
    const errorEmptyMessage = factoryUsingConstructor()
    const errorStaticEmptyMessage = factoryUsingStaticInvoke()
    assertTrue(
        checkMessageAndName(errorEmptyMessage, errorStaticEmptyMessage, "", name),
        `Error in check "${name}" error class with empty message`
    )
    const errorWithMessage = factoryUsingConstructor(message)
    const errorStaticWithMessage = factoryUsingStaticInvoke(message)
    assertTrue(
        checkMessageAndName(
            errorWithMessage,
            errorStaticWithMessage,
            message,
            name
        ),
        `Error in check "${name}" error class with message: "${message}"`
    )
}

function testCreateError() {
    testErrorHelper(
        (x?: string): Error => new Error(x),
        (x?: string): Error => Error(x),
        "Error",
        "Test Error"
    )
    assertEQ(Error("Test Error", new ErrorOptions("Cause")).cause, "Cause")
}

function testThrowError() {
    try {
        throw new Error("Test throw")
    } catch (err: Error) {
        return
    }
}

function testErrorMessage() {
    try {
        throw new Error("Error message test")
    } catch (err: Error) {
        assertEQ(err.message, "Error message test")
        return
    }
}

function testErrorWithCause() {
    const options = new ErrorOptions("Test cause")
    const err = new Error("Test error with cause", options)
    const errSc = Error("Test error with cause", options)
    assertEQ(errSc.cause, err.cause)
    assertEQ(err.cause, options.cause)
}

function testCreateAggregateError() {
    const errors = [new Error("Error 1"), new Error("Error 2")]
    let aggErr = new AggregateError(errors, "Aggregate error message")
    assertEQ(aggErr.errors.length, 2)
    assertEQ(aggErr.message, "Aggregate error message")

    // Note(ivan-tyulyandin): the code below has to moved into generated tests
    // after internal issue #20142 is resolved
    const errInArr = [new Error()]
    const emptyStr = ""
    const nonEmptyStr = "text"
    const errOpts = new ErrorOptions()
    const assertMsg = "length of errors has to be 1"

    // AggregateError(Error)
    aggErr = new AggregateError(errInArr)
    assertEQ(aggErr.errors.length,  1, assertMsg)
    assertEQ<Error, Error>(errInArr[0], aggErr.errors[0])

    // AggregateError(Error, undefined)
    aggErr = new AggregateError(errInArr, undefined)
    assertEQ(aggErr.errors.length,  1, assertMsg)
    assertEQ<Error, Error>(errInArr[0], aggErr.errors[0])
    assertEQ<string, object | undefined>(emptyStr, aggErr.message)

    // AggregateError(Error, undefined, undefined)
    aggErr = new AggregateError(errInArr, undefined)
    assertEQ(aggErr.errors.length,  1, assertMsg)
    assertEQ<Error, Error>(errInArr[0], aggErr.errors[0])
    assertEQ<string, object | undefined>(emptyStr, aggErr.message)
    assertEQ<undefined, object | undefined>(undefined, aggErr.cause)

    // AggregateError(Error, "")
    aggErr = new AggregateError(errInArr, emptyStr)
    assertEQ(aggErr.errors.length,  1, assertMsg)
    assertEQ<Error, Error>(errInArr[0], aggErr.errors[0])
    assertEQ<string, string>(emptyStr, aggErr.message)

    // AggregateError(Error, "text")
    aggErr = new AggregateError(errInArr, nonEmptyStr)
    assertEQ(aggErr.errors.length,  1, assertMsg)
    assertEQ<Error, Error>(errInArr[0], aggErr.errors[0])
    assertEQ<string, string>(nonEmptyStr, aggErr.message)

    // AggregateError(Error, "", undefined)
    aggErr = new AggregateError(errInArr, emptyStr)
    assertEQ(aggErr.errors.length,  1, assertMsg)
    assertEQ<Error, Error>(errInArr[0], aggErr.errors[0])
    assertEQ<string, string>(emptyStr, aggErr.message)
    assertEQ<undefined, object | undefined>(undefined, aggErr.cause)

    // AggregateError(Error, "text", undefined)
    aggErr = new AggregateError(errInArr, nonEmptyStr, undefined)
    assertEQ(aggErr.errors.length,  1, assertMsg)
    assertEQ<Error, Error>(errInArr[0], aggErr.errors[0])
    assertEQ<string, string>(nonEmptyStr, aggErr.message)
    assertEQ<undefined, object | undefined>(undefined, aggErr.cause)

    // AggregateError(Error, undefined, ErrorOptions)
    aggErr = new AggregateError(errInArr, undefined, errOpts)
    assertEQ(aggErr.errors.length,  1, assertMsg)
    assertEQ<Error, Error>(errInArr[0], aggErr.errors[0])
    assertEQ<string, object | undefined>(emptyStr, aggErr.message)
    assertEQ<object | undefined, object | undefined>(errOpts.cause, aggErr.cause)

    // AggregateError(Error, "", ErrorOptions)
    aggErr = new AggregateError(errInArr, emptyStr, errOpts)
    assertEQ(aggErr.errors.length,  1, assertMsg)
    assertEQ<Error, Error>(errInArr[0], aggErr.errors[0])
    assertEQ<string, string>(emptyStr, aggErr.message)
    assertEQ<object | undefined, object | undefined>(errOpts.cause, aggErr.cause)

    // AggregateError(Error, "text", ErrorOptions)
    aggErr = new AggregateError(errInArr, nonEmptyStr, errOpts)
    assertEQ(aggErr.errors.length,  1, assertMsg)
    assertEQ<Error, Error>(errInArr[0], aggErr.errors[0])
    assertEQ<string, string>(nonEmptyStr, aggErr.message)
    assertEQ<object | undefined, object | undefined>(errOpts.cause, aggErr.cause)
}

function testCreateAggregateErrorFromArray() {
    const errors = Array.from<Error>([
        new Error("Error 1"),
        new Error("Error 2")
    ])
    const aggErr = new AggregateError(errors, "Aggregate error message")
    assertEQ(aggErr.errors.length, 2)
    assertEQ(aggErr.message, "Aggregate error message")
}

function testAggregateErrorMessage() {
    const aggErr = new AggregateError(
        [] as Error[],
        "Test aggregate error message"
    )
    assertEQ(aggErr.message, "Test aggregate error message")
}

function testEvalError() {
    testErrorHelper(
        (x?: string): Error => new EvalError(x),
        (x?: string): Error => EvalError(x),
        "EvalError",
        "Test EvalError"
    )
    assertEQ(
        EvalError("Test EvalError", new ErrorOptions("Test cause")).cause,
        "Test cause"
    )
}

function testRangeError() {
    testErrorHelper(
        (x?: string): Error => new RangeError(x),
        (x?: string): Error => RangeError(x),
        "RangeError",
        "Test RangeError"
    )
    assertEQ(
        RangeError("Test RangeError", new ErrorOptions("Test cause")).cause,
        "Test cause"
    )
}

function testReferenceError() {
    testErrorHelper(
        (x?: string): Error => new ReferenceError(x),
        (x?: string): Error => ReferenceError(x),
        "ReferenceError",
        "Test ReferenceError"
    )
    assertEQ(
        ReferenceError("Test ReferenceError", new ErrorOptions("Test cause")).cause,
        "Test cause"
    )
}

function testSyntaxError() {
    testErrorHelper(
        (x?: string): Error => new SyntaxError(x),
        (x?: string): Error => SyntaxError(x),
        "SyntaxError",
        "Test SyntaxError"
    )
    assertEQ(
        SyntaxError("Test SyntaxError", new ErrorOptions("Test cause")).cause,
        "Test cause"
    )
}

function testTypeError() {
    testErrorHelper(
        (x?: string): Error => new TypeError(x),
        (x?: string): Error => TypeError(x),
        "TypeError",
        "Test TypeError"
    )
    assertEQ(
        TypeError("Test TypeError", new ErrorOptions("Test cause")).cause,
        "Test cause"
    )
}

function testURIError() {
    testErrorHelper(
        (x?: string): Error => new URIError(x),
        (x?: string): Error => URIError(x),
        "URIError",
        "Test URIError"
    )
    assertEQ(
        URIError("Test URIError", new ErrorOptions("Test cause")).cause,
        "Test cause"
    )
}
