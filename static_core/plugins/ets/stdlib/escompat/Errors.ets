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

/**
 * @class Represents an error that occurs when global eval() function fails
 */
export class EvalError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("EvalError", message, options)
    }

    static $_invoke(message?: String, options?: ErrorOptions): EvalError {
        return new EvalError(message, options)
    }
}

/**
 * @class Represents an error that occurs when an operation could not be performed, typically (but not exclusively) when a value is not of the expected type
 */
export class TypeError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("TypeError", message, options)
    }

    static $_invoke(message?: String, options?: ErrorOptions): TypeError {
        return new TypeError(message, options)
    }
}

/**
 * @class Represents an error that occurs when a variable that doesn't exist (or hasn't yet been initialized) in the current scope is referenced
 */
export class ReferenceError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("ReferenceError", message, options)
    }

    static $_invoke(message?: String, options?: ErrorOptions): ReferenceError {
        return new ReferenceError(message, options)
    }
}

/**
 * @class Represents an error that occurs when a global URI handling function was used in a wrong way
 */
export class URIError extends Error {
    constructor(message?: String, options?: ErrorOptions) {
        super("URIError", message, options)
    }

    static $_invoke(message?: String, options?: ErrorOptions): URIError {
        return new URIError(message, options)
    }
}

/**
 * The AggregateError object represents an error when several errors
 * need to be wrapped in a single error.
 */
export class AggregateError extends Error {
    errors: Array<Error>

    /**
     * The AggregateError() constructor creates AggregateError
     * object.
     *
     * @param errors An iterable of errors, may not actually be Error
     * instances.
     *
     * @param s An human-readable description of the aggregate error.
     *
     * @param cause A property indicating the specific cause of the
     * error. When catching and re-throwing an error with a
     * more-specific or useful error message, this property can be
     * used to pass the original error.
     *
     */
    constructor(errors: Iterable<Error>, message?: String, options?: ErrorOptions) {
        super("AggregateError", message, options)
        this.errors = Array.from<Error>(errors)
    }

    constructor(errors: Error[], message?: String, options?: ErrorOptions) {
        super("AggregateError", message, options)
        this.errors = Array.from<Error>(errors)
    }

    static $_invoke(errors: Iterable<Error>, message?: String, options?: ErrorOptions): AggregateError {
        return new AggregateError(errors, message, options)
    }

    static $_invoke(errors: Error[], message?: String, options?: ErrorOptions): AggregateError {
        return new AggregateError(errors, message, options)
    }

}
