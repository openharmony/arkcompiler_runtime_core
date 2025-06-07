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

package escompat;

export class ErrorOptions {
    cause: Object | undefined

    constructor(cause?: Object) {
        this.cause = cause
    }
}

/**
 * Stores information about stacktrace and cause in case of an error.
 * Serves as a base class for all error classes.
 */
export class Error {
    private name_: string
    private message_: string
    private stackLines: StackTraceElement[] = []
    private stack_?: String
    private cause_?: Object
    private code_: number

    /**
    * Constructs a new error instance with provided message and cause
    *
    * @param message message of the error
    *
    * @param options options of the error
    */
    constructor(message?: String, options?: ErrorOptions) {
        this("Error", 0 as number, message, options)
    }

    /**
     * Constructs a new error instance with provided code, message and cause
     *
     * @param code code of the error
     *
     * @param message message of the error
     *
     * @param options options of the error
     */
    constructor(code: number, message?: String, options?: ErrorOptions) {
        this("Error", code, message, options)
    }

    /**
    * Constructs a new error instance with provided message, options and name
    *
    * @param name name of the error
    *
    * @param message message of the error
    *
    * @param options options of the error
    */
    constructor(name: String, message: String | undefined, options: ErrorOptions | undefined) {
        this(name, 0 as number, message, options)
    }

    /**
     * Constructs a new error instance with provided code, message, options and name
     *
     * @param name name of the error
     *
     * @param code code of the error
     *
     * @param message message of the error
     *
     * @param options options of the error
     */
    constructor(name: String, code: number, message: String | undefined, options: ErrorOptions | undefined) {
        this.code_ = code
        this.message_ = (message == undefined) ? "" : message
        this.cause_ = (options == undefined) ? undefined : options.cause
        this.name_ = name
        this.stackLines = StackTrace.provisionStackTrace()
    }

    /**
     * Gets the name of the error.
     * @returns The error name.
     */
    get name(): string {
        return this.name_
    }

    /**
     * Sets the name of the error.
     * @param val The new name of the error.
     */
    set name(val: string) {
        this.name_ = val
    }

    /**
     * Gets the code of the error.
     * @returns The error code.
     */
    get code(): number {
        return this.code_
    }

    /**
     * Sets the code of the error.
     * @param val The new code of the error.
     */
    set code(val: number) {
        this.code_ = val
    }

    /**
     * Gets the message of the error.
     * @returns The error message.
     */
    get message(): string {
        return this.message_
    }

    /**
     * Sets the message of the error.
     * @param val The new message of the error.
     */
    set message(val: string) {
        this.message_ = val
    }

    /**
     * Gets the cause of the error.
     * @returns The cause object.
     */
    get cause(): Object | undefined {
        return this.cause_
    }

    /**
     * Sets the cause of the error.
     * @param val The cause object.
     */
    set cause(val: Object | undefined) {
        this.cause_ = val
    }

    static $_invoke(message?: String, options?: ErrorOptions): Error {
        return new Error(message, options)
    }

    /**
    * Converts this error to a string
    * Result includes error message and the stacktrace
    *
    * @returns result of the conversion
    */
    override toString(): String {
        return this.name + ((this.name !== "" && this.message)? ": " + this.message: this.message);
    }

    /**
    * Forms stack and returns it
    */
    get stack(): String | undefined {
        this.formStack()
        return this.stack_
    }

    /**
    * Cleans up stack lines
    */
    set stack(newStack: String | undefined) {
        this.stack_ = newStack
        this.stackLines = []
    }

    /**
    * Forms stack from this.stackLines and stores it in this.stack_
    */
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

