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

package escompat;

export class ErrorOptions {
    cause: Object | undefined = undefined

    constructor(cause: Object | undefined) {
        this.cause = cause
    }
}

/**
 * Strores information about stacktrace and cause in case of an error.
 * Serves as a base class for all error classes.
 */
export class Error {
    cause: Object | undefined
    message: String
    name: String
    private stackLines: StackTraceElement[] = []
    private stack_: String | undefined = undefined

    /**
    * Constructs a new error instance with provided message and cause
    *
    * @param message message of the error
    *
    * @param options options of the error
    */
    constructor(message?: String, options?: ErrorOptions) {
        this("Error", message, options)
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
        this.message = (message == undefined) ? "" : message
        this.cause = (options == undefined) ? undefined : options.cause
        this.name = name
        this.stackLines = StackTrace.provisionStackTrace()
    }

    static invoke(message?: String, options?: ErrorOptions): Error {
        return new Error(message, options)
    }

    static invoke(message: String): Error {
        return new Error(message)
    }

    /**
    * Converts this error to a string
    * Result includes error message and the stacktrace
    *
    * @returns result of the conversion
    */
    override toString(): String {
        if (this.message != "") {
            return this.name + ": " + this.message;
        }
        return this.name;
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

        if (this.message != "") {
            let val = this.name + ": " + this.message + "\n"
            builder.append(val)
        }

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

