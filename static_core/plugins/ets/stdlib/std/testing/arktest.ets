/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
/**
 * @file Defines the arktest for ArkTS
 * @kit ArkTS
 */

package std.testing;

/**
 * Provide a namespace for ArkTS testing-related functionality.
 * @namespace
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export namespace arktest {
    /**
    * Produce AssertionError if condition is false. Print an additinal message in this case
    * @param {boolean} condition The provided condition.
    * @param {string}  message Optional comment printed when the assertion fails.
    * @param {string} [description] description comment printed when the assertion fails.
    * @throws {AssertionError} Thrown when the condition is false.
    * @syscap SystemCapability.Utils.Lang
    * @FaAndStageModel
    * @since 24
    */
    function assertCommon(condition: boolean, message: string, description?: string): void {
        if (!condition) {
            failingAssertion(message, description)
        }
    }

    /**
    * Produce AssertionError unconditionally
    * @param {string}  message Optional comment printed when the assertion fails.
    * @param {string} [description] description comment printed when the assertion fails.
    * @throws {AssertionError} Thrown when the condition is false.
    * @syscap SystemCapability.Utils.Lang
    * @FaAndStageModel
    * @since 24
    */
    function failingAssertion(message: string, description?: string): never {
        throw new AssertionError((description ? description + "\n" : "") + message)
    }

    /**
    * Asserts that the provided condition is true.
    * @param {boolean} condition The provided condition.
    * @param {string} [comment] Optional comment printed when the assertion fails.
    * @throws {AssertionError} Thrown when the condition is false.
    * @syscap SystemCapability.Utils.Lang
    * @FaAndStageModel
    * @since 24
    */
    export function assertTrue(condition: boolean, comment?: string): void {
        assertCommon(condition, "expected true but was false", comment)
    }

    /**
    * Asserts that the provided condition is false.
    * @param {boolean} condition The provided condition.
    * @param {string} [comment] Optional comment printed when the assertion fails.
    * @throws {AssertionError} Thrown when the condition is true.
    * @syscap SystemCapability.Utils.Lang
    * @FaAndStageModel
    * @since 24
    */
    export function assertFalse(condition: boolean, comment?: string): void {
        assertCommon(!condition, "expected false but was true", comment)
    }

    /**
    * Checks whether two values are equal (===). Throws an error if the check fails.
    * @param {T1} value1 The first value to compare.
    * @param {T2} value2 The second value to compare.
    * @param {string} [comment] Optional comment printed when the equality check fails.
    * @throws {AssertionError} Thrown when the two values are not equal.
    * @syscap SystemCapability.Utils.Lang
    * @FaAndStageModel
    * @since 24
    */
    export function assertEQ<T1, T2>(value1: T1, value2: T2, comment?: string): void {
        assertCommon(value1 === value2, "expected equality failed: '" + value1 + "' === '" + value2 + "'", comment);
    }

    /**
    * Checks whether two values are not equal (!==). Throws an error if the check fails.
    * @param {T1} value1 The first value to compare.
    * @param {T2} value2 The second value to compare.
    * @param {string} [comment] Optional comment printed when the non-equality check fails.
    * @throws {AssertionError} Thrown when the two values are equal.
    * @syscap SystemCapability.Utils.Lang
    * @FaAndStageModel
    * @since 24
    */
    export function assertNE<T1, T2>(value1: T1, value2: T2, comment?: string): void {
        assertCommon(value1 !== value2, "expected non-equality failed: '" + value1 + "' !== '" + value2 + "'", comment);
    }

    /**
    * Checks whether the difference between two double values does not exceed the specified absolute error.
    * @param {double} value1 The first value to be checked.
    * @param {double} value2 The second value to be checked.
    * @param {double} absError The allowed absolute error.
    * @param {string} [comment] Optional comment printed when the check fails.
    * @throws {AssertionError} Thrown when the difference between the two values exceeds the absolute error.
    * @syscap SystemCapability.Utils.Lang
    * @FaAndStageModel
    * @since 24
    */
    export function assertDoubleEQ(value1: double, value2: double, absError: double, comment?: string): void {
        let difference = Math.abs(value1 - value2);
        assertCommon(difference <= absError, "The difference between '" + value1 + "' and '" + value2 +
                                            "' is '" + difference + "', but expected absolute error = '" + absError + "'",
                                            comment);
    }

    /**
    * Checks whether the first value is less than the second value. Throws an error if the check fails.
    * @param {byte} value1 The first value to compare.
    * @param {byte} value2 The second value to compare.
    * @param {string} [comment] Optional comment printed when the check fails.
    * @throws {AssertionError} Thrown when the first value is not less than the second value.
    * @syscap SystemCapability.Utils.Lang
    * @FaAndStageModel
    * @since 24
    */
    export function assertLT(value1: byte, value2: byte, comment?: string): void {
        assertCommon(value1 < value2, "expected non-equality failed: '" + value1 + "' < '" + value2 + "'", comment);
    }

    /**
    * Checks whether the first value is less than the second value. Throws an error if the check fails.
    * @param {short} value1 The first value to compare.
    * @param {short} value2 The second value to compare.
    * @param {string} [comment] Optional comment printed when the check fails.
    * @throws {AssertionError} Thrown when the first value is not less than the second value.
    * @syscap SystemCapability.Utils.Lang
    * @FaAndStageModel
    * @since 24
    */
    export function assertLT(value1: short, value2: short, comment?: string): void {
        assertCommon(value1 < value2, "expected non-equality failed: '" + value1 + "' < '" + value2 + "'", comment);
    }

    /**
    * Checks whether the first value is less than the second value. Throws an error if the check fails.
    * @param {int} value1 The first value to compare.
    * @param {int} value2 The second value to compare.
    * @param {string} [comment] Optional comment printed when the check fails.
    * @throws {AssertionError} Thrown when the first value is not less than the second value.
    * @syscap SystemCapability.Utils.Lang
    * @FaAndStageModel
    * @since 24
    */
    export function assertLT(value1: int, value2: int, comment?: string): void {
        assertCommon(value1 < value2, "expected non-equality failed: '" + value1 + "' < '" + value2 + "'", comment);
    }

    /**
    * Checks whether the first value is less than the second value. Throws an error if the check fails.
    * @param {long} value1 The first value to compare.
    * @param {long} value2 The second value to compare.
    * @param {string} [comment] Optional comment printed when the check fails.
    * @throws {AssertionError} Thrown when the first value is not less than the second value.
    * @syscap SystemCapability.Utils.Lang
    * @FaAndStageModel
    * @since 24
    */
    export function assertLT(value1: long, value2: long, comment?: string): void {
        assertCommon(value1 < value2, "expected non-equality failed: '" + value1 + "' < '" + value2 + "'", comment);
    }

    /**
    * Checks whether the first value is less than the second value. Throws an error if the check fails.
    * @param {float} value1 The first value to compare.
    * @param {float} value2 The second value to compare.
    * @param {string} [comment] Optional comment printed when the check fails.
    * @throws {AssertionError} Thrown when the first value is not less than the second value.
    * @syscap SystemCapability.Utils.Lang
    * @FaAndStageModel
    * @since 24
    */
    export function assertLT(value1: float, value2: float, comment?: string): void {
        assertCommon(value1 < value2, "expected non-equality failed: '" + value1 + "' < '" + value2 + "'", comment);
    }

    /**
    * Checks whether the first value is less than the second value. Throws an error if the check fails.
    * @param {double} value1 The first value to compare.
    * @param {double} value2 The second value to compare.
    * @param {string} [comment] Optional comment printed when the check fails.
    * @throws {AssertionError} Thrown when the first value is not less than the second value.
    * @syscap SystemCapability.Utils.Lang
    * @FaAndStageModel
    * @since 24
    */
    export function assertLT(value1: double, value2: double, comment?: string): void {
        assertCommon(value1 < value2, "expected non-equality failed: '" + value1 + "' < '" + value2 + "'", comment);
    }

    /**
    * Checks whether the first value is less than or equal to the second value. Throws an error if the check fails.
    * @param {byte} value1 The first value to compare.
    * @param {byte} value2 The second value to compare.
    * @param {string} [comment] Optional comment printed when the check fails.
    * @throws {AssertionError} Thrown when the first value is greater than the second value.
    * @syscap SystemCapability.Utils.Lang
    * @FaAndStageModel
    * @since 24
    */
    export function assertLE(value1: byte, value2: byte, comment?: string): void {
        assertCommon(value1 <= value2, "expected non-equality failed: '" + value1 + "' <= '" + value2 + "'", comment);
    }

    /**
    * Checks whether the first value is less than or equal to the second value. Throws an error if the check fails.
    * @param {short} value1 The first value to compare.
    * @param {short} value2 The second value to compare.
    * @param {string} [comment] Optional comment printed when the check fails.
    * @throws {AssertionError} Thrown when the first value is greater than the second value.
    * @syscap SystemCapability.Utils.Lang
    * @FaAndStageModel
    * @since 24
    */
    export function assertLE(value1: short, value2: short, comment?: string): void {
        assertCommon(value1 <= value2, "expected non-equality failed: '" + value1 + "' <= '" + value2 + "'", comment);
    }

    /**
    * Checks whether the first value is less than or equal to the second value. Throws an error if the check fails.
    * @param {int} value1 The first value to compare.
    * @param {int} value2 The second value to compare.
    * @param {string} [comment] Optional comment printed when the check fails.
    * @throws {AssertionError} Thrown when the first value is greater than the second value.
    * @syscap SystemCapability.Utils.Lang
    * @FaAndStageModel
    * @since 24
    */
    export function assertLE(value1: int, value2: int, comment?: string): void {
        assertCommon(value1 <= value2, "expected non-equality failed: '" + value1 + "' <= '" + value2 + "'", comment);
    }

    /**
    * Checks whether the first value is less than or equal to the second value. Throws an error if the check fails.
    * @param {long} value1 The first value to compare.
    * @param {long} value2 The second value to compare.
    * @param {string} [comment] Optional comment printed when the check fails.
    * @throws {AssertionError} Thrown when the first value is greater than the second value.
    * @syscap SystemCapability.Utils.Lang
    * @FaAndStageModel
    * @since 24
    */
    export function assertLE(value1: long, value2: long, comment?: string): void {
        assertCommon(value1 <= value2, "expected non-equality failed: '" + value1 + "' <= '" + value2 + "'", comment);
    }

    /**
    * Checks whether the first value is less than or equal to the second value. Throws an error if the check fails.
    * @param {float} value1 The first value to compare.
    * @param {float} value2 The second value to compare.
    * @param {string} [comment] Optional comment printed when the check fails.
    * @throws {AssertionError} Thrown when the first value is greater than the second value.
    * @syscap SystemCapability.Utils.Lang
    * @FaAndStageModel
    * @since 24
    */
    export function assertLE(value1: float, value2: float, comment?: string): void {
        assertCommon(value1 <= value2, "expected non-equality failed: '" + value1 + "' <= '" + value2 + "'", comment);
    }

    /**
    * Checks whether the first value is less than or equal to the second value. Throws an error if the check fails.
    * @param {double} value1 The first value to compare.
    * @param {double} value2 The second value to compare.
    * @param {string} [comment] Optional comment printed when the check fails.
    * @throws {AssertionError} Thrown when the first value is greater than the second value.
    * @syscap SystemCapability.Utils.Lang
    * @FaAndStageModel
    * @since 24
    */
    export function assertLE(value1: double, value2: double, comment?: string): void {
        assertCommon(value1 <= value2, "expected non-equality failed: '" + value1 + "' <= '" + value2 + "'", comment);
    }

    /**
    * Asserts that a callback function throws an exception and tests the thrown value.
    * @param {Function} cb The callback function to be executed.
    * @param {Function} [test] A function used to test the thrown value, 
    *     acting as a predicate or returning an error message.
    * @throws {AssertionError} Thrown when the callback function does not throw a value 
    *     or when the thrown value does not satisfy the test predicate.
    * @syscap SystemCapability.Utils.Lang
    * @FaAndStageModel
    * @since 24
    */
    export function expectThrow(
        cb: () => void,
        test?: (v: Error) => boolean | string) {
        try {
            cb()
        } catch (e) {
            e = (e instanceof Error) ? e as Error
                : failingAssertion("An arbitrary object instance was caught!")
            let res = test?.(e)
            if (res != undefined && res != true) {
                failingAssertion((typeof res == "string") ? res as string : ("Unexpected value thrown: " + e))
            }
            return
        }
        failingAssertion("Callback expected to throw");
    }

    /**
    * Executes the callback function and checks whether an Error occurs. If an expected value is provided, 
    * also checks whether the thrown error matches the expected value.
    * @param {Function} fn The callback function to be executed.
    * @param {Function} [expect] An optional string message or Error instance used to test the thrown value.
    * @throws {AssertionError} Thrown when no error occurs or when the thrown error does not match the expected error.
    * @syscap SystemCapability.Utils.Lang
    * @FaAndStageModel
    * @since 24
    */
    export function expectError(fn: () => void, expect?: string | Error): void {
        expectThrow(fn, (e) => {
            if (!(e instanceof Error)) {
                return "Expected an instance of Error, got " + Class.of(e).getName()
            }
            if (expect === undefined) {
                return true
            } else if (typeof expect == "string") {
                return e.message == expect
            } else {
                const expectedError = expect as Error // broken smartcast
                const eType = Class.of(e)
                const expectType = Class.of(expectedError)
                if (eType !== expectType) {
                    return "Expected " + expectType.getName() + ", got " + eType.getName()
                }
                if (e.message != expectedError.message) {
                    return "Expected '" + expectedError.message + "', got '" + e.message + "'"
                }
                return true
            }
        })
    }

    /**
    * Executes the callback function and checks that no error occurs.
    * @param {Function} fn The callback function to be executed.
    * @throws {AssertionError} Thrown when an error occurs.
    * @syscap SystemCapability.Utils.Lang
    * @FaAndStageModel
    * @since 24
    */
    export function expectNoThrow(fn: () => void): void {
        try {
            fn();
        } catch (e) {
            assertCommon(false, "Error is not expected, but has ocurred: " + e);
        }
    }

    /**
     * Represents terminal colors.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    class TermColor {
        /**
         * Represents the color black.
         * @type {TermColor}
         * @readonly
         * @static
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         * @since 24
         */
        static readonly BLACK = new TermColor(0)

        /**
         * Represents the color red.
         * @type {TermColor}
         * @readonly
         * @static
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         * @since 24
         */
        static readonly RED = new TermColor(1)

        /**
         * Represents the color green.
         * @type {TermColor}
         * @readonly
         * @static
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         * @since 24
         */
        static readonly GREEN = new TermColor(2)

        /**
         * The color code.
         * @type {int}
         * @readonly
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         * @since 24
         */
        readonly code: int

        private constructor(code: int) {
            this.code = code
        }

        /**
         * Returns the string representation of the color.
         * @returns {string}
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         * @since 24
         */
        toString(): string {
            return "" + this.code
        }
    }

    /**
     * Provides an interface for running a single test.
     * @class ArkTest Provides an interface for running a test.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    export class ArkTest {
        /**
         * Creates a test instance.
         * @param {string} name The test name.
         * @param {Function} callback The callback function containing the test body.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         * @since 24
         */
        constructor(name: string, callback: () => void) {
            this.name = name;
            this.callback = callback;
        }

        /**
         * Runs this test.
         * @returns {boolean} 
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         * @since 24
         */
        final run(): boolean {
            let testcaseName: string = this.getFullName();
            let startMs = Date.now();
            try {
                this.callback();
            } catch (e) {
                const failedMsg = this.addTermColor(TermColor.RED, "FAILED")
                const msgFail = "[ " + failedMsg + " ] " + testcaseName + " (" + (Date.now() - startMs) + " ms)";
                if (!(e instanceof AssertionError)) {
                    console.error("Unexpected error: " + e);
                } else {
                    // AssertionError already print message in constructor console.error,
                    // so set message to empty to avoid double printing
                    e.message = "";
                }
                if (e instanceof Error) {
                    if (e.stack) {
                        console.error(e.stack);
                    }
                }
                console.log(msgFail);
                return false;
            }
            const passedMsg = this.addTermColor(TermColor.GREEN, "PASSED")
            console.log("[ " + passedMsg + " ] " + testcaseName + " (" + (Date.now() - startMs) + " ms)");
            return true;
        }

        private addTermColor(color: TermColor, str: string): string {
            return "\x1b[38;5;" + color + "m" + str + "\x1b[0m"
        }

        /**
         * Gets the full name of the test (including the test suite name).
         * @returns {string} The full test name.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         * @since 24
         */
        final getFullName(): string {
            let name: string = "";
            if (this.suite) {
                name += this.suite!.name + ".";
            }
            return name + this.name;
        }

        /**
         * Sets the test suite this test belongs to.
         * @param {ArkTestsuite} suite The test suite instance.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         * @since 24
         */
        public setSuite(suite: ArkTestsuite): void {
            this.suite = suite;
        }

        /**
         * The test name.
         * @type {string}
         * @readonly
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         * @since 24
         */
        readonly name: string;

        private readonly callback: () => void;
        private suite?: ArkTestsuite;
    }

    /**
     * Provides an interface for running a group of tests.
     * @class ArkTestsuite provides interface for running a group of tests
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    export class ArkTestsuite {

       /**
         * Creates a test suite instance.
         * @param {string} name The test suite name.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         * @since 24
         */
        constructor(name: string) {
            this.name = name;
            this.tests = new Array<ArkTest>();
        }

        /**
         * Executed before each test runs. Can be overridden in derived classes.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         * @since 24
         */
        before(): void {}

        /**
         * Executed after each test runs. Can be overridden in derived classes.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         * @since 24
         */
        after(): void {}

        /**
         * Adds a new test to the test suite.
         * @param {string} name The test name.
         * @param {Function} callback The callback function containing the test body.
         * @throws {Error} Thrown when the test name is empty.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         * @since 24
         */
        final addTest(name: string, callback: () => void): void {
            this.addTest(new ArkTest(name, callback));
        }

        /**
         * Adds a new asynchronous test to the test suite.
         * @param {string} name The asynchronous test name.
         * @param {Function} asyncCallback The callback function containing the asynchronous test body.
         * @throws {Error} Thrown when the test name is empty.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         * @since 24
         */
        final addAsyncTest(name: string, asyncCallback: () => Promise<void>): void {
            let callback = () => {
                waitForCompletion(asyncCallback);
            };
            this.addTest(new ArkTest(name, callback));
        }

        /**
         * Adds a new test instance to the test suite.
         * @param {ArkTest} test The test instance.
         * @throws {Error} Thrown when the test name is empty.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         * @since 24
         */
        final addTest(test: ArkTest): void {
            if (test.name == "") {
                throw new Error("Test with empty name is not allowed");
            }
            test.setSuite(this);
            this.tests.push(test);
        }

        /**
         * Runs all tests in the test suite.
         * @returns {int} The number of failed tests.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         * @since 24
         */
        final run(): int {
            let startMs = Date.now();
            this.tests.forEach((test: ArkTest) => {
                this.before();
                if (test.run()) {
                    this.passedTests += 1;
                } else {
                    this.failedTests += 1;
                }
                this.after();
            });
            let execTimeMs: string = "(" + (Date.now() - startMs) + " ms)";
            let failedTestsCount = this.failedTests;
            if (failedTestsCount != 0) {
                console.log("Passed tests: " + this.passedTests);
                console.log("Failed tests: " + this.failedTests);
                console.log(this.name + " testsuite failed " + execTimeMs);
            } else {
                let testStr: string = this.passedTests + ((this.passedTests == 1) ? " test" : " tests");
                console.log(testStr + " from " +  this.name + " testsuite passed " + execTimeMs);
            }
            this.failedTests = 0;
            this.passedTests = 0;
            return failedTestsCount;
        }

        /**
         * The name of the test suite.
         * @type {string}
         * @readonly
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         * @since 24
         */
        readonly name: string;

        private tests: Array<ArkTest>;
        private passedTests: int = 0;
        private failedTests: int = 0;
    }

}
