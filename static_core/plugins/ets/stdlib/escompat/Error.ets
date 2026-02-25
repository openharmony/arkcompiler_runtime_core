/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
 * @file Defines the Error for ArkTS
 * @kit ArkTS
 */

package escompat;

/**
 * Defines ErrorOptions interface
 *
 * @interface ErrorOptions
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export interface ErrorOptions {
    /**
     * Specifies cause of an Error
     *
     * @type { Object | undefined }
     * @default undefined
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    cause?: Object
}

/**
 * Defines Error class
 *
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export class Error {
    private name_: string
    private message_: string
    private stackLines: FixedArray<StackTraceElement> = []
    private stack_?: String
    private cause_?: Object
    private code_: int

    /**
     * Constructs a new error instance with provided code, message and cause
     *
     * @param { int } code - Error code
     * @param { String } [message] - Error text
     * @param { ErrorOptions } [options] - Error options
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(code: int, message?: String, options?: ErrorOptions) {
        this("Error", code, message, options)
    }

    /**
     * Constructs a new error instance with provided message and cause
     *
     * @param { String } [message] - Error text
     * @param { ErrorOptions } [options] - Error options
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(message?: String, options?: ErrorOptions) {
        this("Error", 0, message, options)
    }

    /**
     * Constructs a new error instance with provided name, code, message and options
     *
     * @param { String } name - Error name
     * @param { int } code - Error code
     * @param { String } [message] - Error text
     * @param { ErrorOptions } [options] - Error options
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(name: String, code: int, message?: String, options?: ErrorOptions) {
        this.code_ = code
        this.message_ = (message == undefined) ? "" : message
        this.cause_ = (options == undefined) ? undefined : options.cause
        this.name_ = name
        this.stackLines = StackTrace.provisionStackTrace()
    }

    /**
     * Constructs a new error instance with provided name, message and options
     *
     * @param { String } name - Error name
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions } [options] - Error options
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(name: String, message: String | undefined, options?: ErrorOptions) {
        this(name, 0, message, options)
    }

    /**
     * Gets the name of the error.
     *
     * @returns { string } - Current error name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    get name(): string {
        return this.name_
    }

    /**
     * Sets the name of the error.
     *
     * @param { string } val - New name for the Error
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    set name(val: string) {
        this.name_ = val
    }

    /**
     * Gets the code of the error.
     *
     * @returns { int } - Current error code
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    get code(): int {
        return this.code_
    }

    /**
     * Sets the code of the error.
     *
     * @param { int } val - New code for the Error
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    set code(val: int) {
        this.code_ = val
    }

    /**
     * Gets the message of the error.
     *
     * @returns { string } - Current error message
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    get message(): string {
        return this.message_
    }

    /**
     * Sets the message of the error.
     *
     * @param { string } val - New message for the Error
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    set message(val: string) {
        this.message_ = val
    }

    /**
     * Gets the cause of the error.
     *
     * @returns { Object | undefined } - Current error cause
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    get cause(): Object | undefined {
        return this.cause_
    }

    /**
     * Sets the cause of the error.
     *
     * @param { Object | undefined } val - New cause for the Error
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    set cause(val: Object | undefined) {
        this.cause_ = val
    }

    /**
     * Constructs a new error instance with provided message and options
     *
     * @param { String } [message] - Error text
     * @param { ErrorOptions } [options] - Error options
     * @returns { Error } - Newly created Error instance
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    static $_invoke(message?: String, options?: ErrorOptions): Error {
        return new Error(message, options)
    }

    /**
     * Converts this error to a string
     *
     * @returns { String } - String representation of the Error
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    override toString(): String {
        return this.name + ((this.name !== "" && this.message) ? ": " + this.message : this.message);
    }

    /**
     * Forms stack and returns it
     *
     * @returns { String | undefined } - Current error stacktrace
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    get stack(): String | undefined {
        this.formStack()
        return this.stack_
    }

    /**
     * Does stack replacement and clears previously collected stack info
     *
     * @param { String | undefined } newStack - New stacktrace for the error
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    set stack(newStack: String | undefined) {
        this.stack_ = newStack
        this.stackLines = []
    }

    private formStack() {
        if (this.stack_ != undefined) {
            return
        }
        if (this.stackLines.length == 0) {
            return
        }
        let builder = new StringBuilder("")

        // NOTE(kparshukov): find a better way to erase Error's ctors lines
        const provisionStackTraceLevel = 2
        const realStackStart = (this.stackLines.length > provisionStackTraceLevel ? provisionStackTraceLevel : 0)
        for (let i: int = realStackStart; i < this.stackLines.length; i++) {
            builder.append(this.stackLines[i].toString() + '\n')
        }

        this.stack_ = builder.toString()
        this.stackLines = []
    }
}

