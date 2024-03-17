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

package std.core;

/**
 * @class Represents an error that occurs when assertion fails.
 */

export class AssertionError extends Error {
    readonly tmp_console = new Console();

    //NOTE(kirill-mitkin): Used by frontend
    constructor(message: String) {
        this(message, undefined)
    }

    constructor(message?: String, options?: ErrorOptions) {
        super("AssertionError", message, options)
        this.tmp_console.log(message!);
    }
}

/**
 * @class Represents an error that occurs when division by zero is performed.
 */
export class DivideByZeroError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("DivideByZeroError", message, options)
    }
}

/**
 * @class Represents an error that occurs when null pointer is dereferenced.
 */
export class NullPointerError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("NullPointerError", message, options)
    }
}

/**
 * @class Represents an error that occurs when exception is thrown and not catched
 */
export class UncatchedExceptionError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("UncatchedExceptionError", message, options)
    }
}

/**
 * @class Represents an error that occurs when trying to interpret syntactically invalid code
 */
export class SyntaxError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("SyntaxError", message, options)
    }
}

/**
 * @class Represents an error that occurs when JSONValue can not be assigned to a type
 */
export class JSONTypeError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("JSONTypeError", message, options)
    }
}

/**
 * @class Represents an error that occurs when provided collection index is out of range
 */
export class RangeError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("RangeError", message, options)
    }
}

/**
 * @class Represents an error that occurs when array is oging to be indexed out of its bounds
 */
export final class ArrayIndexOutOfBoundsError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("ArrayIndexOutOfBoundsError", message, options)
    }
}
