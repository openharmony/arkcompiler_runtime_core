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

package std.core;

/**
 * @class Represents an error that occurs when assertion fails.
 */
export class AssertionError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("AssertionError", message, options)
        console.log(message ?? "Assertion failed.");
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
 * @class Represents an error that occurs when exception is thrown and not caught
 */
export class UncaughtExceptionError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("UncaughtExceptionError", message, options)
    }
}

/**
 * @class Represents an error that occurs when trying to interpret syntactically invalid code
 */
export class SyntaxError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("SyntaxError", message, options)
    }

    static $_invoke(message?: String, options?: ErrorOptions): SyntaxError {
        return new SyntaxError(message, options)
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

    static $_invoke(message?: String, options?: ErrorOptions): RangeError {
        return new RangeError(message, options)
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

/**
 * @class Represents exception that is thrown when illegal arithmetic operation is occured (e.g. division by zero)
 */
export final class ArithmeticError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("ArithmeticError", message, options)
    }
}

/**
 * @class Represents an error that occurs when memory allocation fails
 */
export class OutOfMemoryError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("OutOfMemoryError", message, options)
    }
}

/**
 * @class Represents an error that occurs when the available memory is not sufficient to create the activation frame
 */
export class StackOverflowError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("StackOverflowError", message, options)
    }
}

/**
 * @class Represents an error that occurs when attempting to instantiate abstract class or an interface
 */
export class InstantiationError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("InstantiationError", message, options)
    }
}

/**
 * @class Represents an error that occurs when an internal error occured
 */
export class InternalError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("InternalError", message, options)
    }
}

/**
 * @class Represents an error that is thrown when attempting to wait, notify or notifyAll on object, that hasn't been synchronised
 */
export class IllegalMonitorStateError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("IllegalMonitorStateError", message, options)
    }
}

/**
 * @class Represents exception that is thrown when provided collection index is out of bounds
 */
export class IndexOutOfBoundsError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("IndexOutOfBoundsError", message, options)
    }
}

/**
 * @class Represents exception that is thrown when provided string index is out of bounds
 */
export final class StringIndexOutOfBoundsError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("StringIndexOutOfBoundsError", message, options)
    }
}

/**
 * @class Represents exception that is thrown in case of illegal class casting
 */
export final class ClassCastError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("ClassCastError", message, options)
    }
}

/**
 * @class Represents exception that is thrown when attempting to store an object of different type in array of type-erased objects
 */
export final class ArrayStoreError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("ArrayStoreError", message, options)
    }
}

/**
 * @class Represents exception that is thrown when negative array size is supplied
 */
export final class NegativeArraySizeError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("NegativeArraySizeError", message, options)
    }
}

export class ExceptionInInitializerError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("ExceptionInInitializerError", message, options)
    }
}

export class CoroutinesLimitExceedError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("CoroutinesLimitExceedError", message, options)
    }
}

/**
 * @class Represents the error that is thrown when a requested coroutine operation is invalid in the current context.
 */
export class InvalidCoroutineOperationError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("InvalidCoroutineOperationError", message, options)
    }
}

export class ArgumentsUnderapplicationError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("ArgumentsUnderapplicationError", message, options)
    }
}