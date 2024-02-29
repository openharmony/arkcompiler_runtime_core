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

/**
 * @class Represents an error that occurs when assertion fails.
 */

export class AssertionError extends Error {
    readonly tmp_console = new Console();

    constructor() {
        super();
        this.name = "AssertionError";
    }

    constructor(message: String) {
        super(message);
        this.name = "AssertionError";
        this.tmp_console.print(message);
        this.tmp_console.println();
    }

    constructor(message: String, cause: Object) {
        super(message, cause);
        this.name = "AssertionError";
        this.tmp_console.print(message);
        this.tmp_console.println();
    }
}

/**
 * @class Represents an error that occurs when division by zero is performed.
 */
export class DivideByZeroError extends Error {
    constructor() {
        super();
        this.name = "DivideByZeroError";
    }

    constructor(s: String) {
        super(s);
        this.name = "DivideByZeroError";
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
        this.name = "DivideByZeroError";
    }
}

/**
 * @class Represents an error that occurs when null pointer is dereferenced.
 */
export class NullPointerError extends Error {
    constructor() {
        super();
        this.name = "NullPointerError";
    }

    constructor(s: String) {
        super(s);
        this.name = "NullPointerError";
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
        this.name = "NullPointerError";
    }
}

/**
 * @class Represents an error that occurs when exception is thrown and not catched
 */
export class UncatchedExceptionError extends Error {
    constructor() {
        super();
        this.name = "UncatchedExceptionError";
    }

    constructor(s: String) {
        super(s);
        this.name = "UncatchedExceptionError";
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
        this.name = "UncatchedExceptionError";
    }
}

/**
 * @class Represents an error that occurs when provided collection index is out of range
 */
export class RangeError extends Error {
    constructor() {
        super();
        this.name = "RangeError";
    }

    constructor(s: String) {
        super(s);
        this.name = "RangeError";
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
        this.name = "RangeError";
    }
}

/**
 * @class Represents an error that occurs when memory allocation fails
 */
export class OutOfMemoryError extends Error {
    constructor() {
        super();
        this.name = "OutOfMemoryError";
    }

    constructor(s: String) {
        super(s);
        this.name = "OutOfMemoryError";
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
        this.name = "OutOfMemoryError";
    }
}

/**
 * @class Represents an error that occurs when no such method is found in the class
 */
export class NoSuchMethodError extends Error {
    constructor() {
        super();
        this.name = "NoSuchMethodError";
    }

    constructor(s: String) {
        super(s);
        this.name = "NoSuchMethodError";
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
        this.name = "NoSuchMethodError";
    }
}

/**
 * @class Represents an error that occurs when the available memory is not sufficient to create the activation frame
 */
export class StackOverflowError extends Error {
    constructor() {
        super();
        this.name = "StackOverflowError";
    }

    private constructor(s: String) {}

    private constructor(s: String, cause: Object) {}
}

/**
 * @class Represents an error that occurs when attempting to instantiate abstract class or an interface
 */
export class InstantiationError extends Error {
    constructor() {
        super();
        this.name = "InstantiationError";
    }

    constructor(s: String) {
        super(s);
        this.name = "InstantiationError";
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
        this.name = "InstantiationError";
    }
}

/**
 * @class Represents an error that occurs when an internal error occured
 */
export class InternalError extends Error {
    constructor() {
        super();
        this.name = "InternalError";
    }

    constructor(s: String) {
        super(s);
        this.name = "InternalError";
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
        this.name = "InternalError";
    }
}

/**
 * @class Represents an error that occurs when no class definition has been found
 */
export class NoClassDefFoundError extends Error {
    constructor() {
        super();
        this.name = "NoClassDefFoundError";
    }

    constructor(s: String) {
        super(s);
        this.name = "NoClassDefFoundError";
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
        this.name = "NoClassDefFoundError";
    }
}

/**
 * @class Represents an error that occurs when no such field is found in the class
 */
export class NoSuchFieldError extends Error {
    constructor() {
        super();
        this.name = "NoSuchFieldError";
    }

    constructor(s: String) {
        super(s);
        this.name = "NoSuchFieldError";
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
        this.name = "NoSuchFieldError";
    }
}

/**
 * @class Represents an error that occurs when global eval() function fails
 */
export class EvalError extends Error {
    constructor() {
        super();
        this.name = "EvalError";
    }

    constructor(s: String) {
        super(s);
        this.name = "EvalError";
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
        this.name = "EvalError";
    }
}

/**
 * @class Represents an error that occurs when a variable that doesn't exist (or hasn't yet been initialized) in the current scope is referenced
 */
export class ReferenceError extends Error {
    constructor() {
        super();
        this.name = "ReferenceError";
    }

    constructor(s: String) {
        super(s);
        this.name = "ReferenceError";
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
        this.name = "ReferenceError";
    }
}

/**
 * @class Represents an error that occurs when trying to interpret syntactically invalid code
 */
export class SyntaxError extends Error {
    constructor() {
        super();
        this.name = "SyntaxError";
    }

    constructor(s: String) {
        super(s);
        this.name = "SyntaxError";
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
        this.name = "SyntaxError";
    }
}

/**
 * @class Represents an error that occurs when an operation could not be performed, typically (but not exclusively) when a value is not of the expected type
 */
export class TypeError extends Error {
    constructor() {
        super();
        this.name = "TypeError";
    }

    constructor(s: String) {
        super(s);
        this.name = "TypeError";
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
        this.name = "TypeError";
    }
}

/**
 * @class Represents an error that occurs when a global URI handling function was used in a wrong way
 */
export class URIError extends Error {
    constructor() {
        super();
        this.name = "URIError";
    }

    constructor(s: String) {
        super(s);
        this.name = "URIError";
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
        this.name = "URIError";
    }
}

/**
 * @class Represents an error that occurs when JSONValue can not be assigned to a type
 */
export class JSONTypeError extends Error {
    constructor() {
        super();
        this.name = "JSONTypeError";
    }

    constructor(s: String) {
        super(s);
        this.name = "JSONTypeError";
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
        this.name = "JSONTypeError";
    }
}

/**
 * The AggregateError object represents an error when several errors
 * need to be wrapped in a single error.
 */
export class AggregateError extends Error {
    errors: Error[] = [];

    /**
     * The AggregateError() constructor creates AggregateError
     * object.
     *
     * @param errors An iterable of errors, may not actually be Error instances.
     */
    constructor(errors: Error[]) {
        super();
        this.errors = errors;
        this.name = "AggregateError";
    }

    /**
     * The AggregateError() constructor creates AggregateError
     * object with message given.
     *
     * @param errors An iterable of errors, may not actually be Error instances.
     *
     * @param s An human-readable description of the aggregate error.
     */
    constructor(errors: Error[], s: String) {
        super(s);
        this.errors = errors;
        this.name = "AggregateError";
    }

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
    constructor(errors: Error[], s: String, cause: Object) {
        super(s, cause);
        this.errors = errors;
        this.name = "AggregateError";
    }
}

/**
 * The AbstractMethodError object represents an error that occures when abstract method get called.
 */
export class AbstractMethodError extends Error {
    constructor() {
        super();
        this.name = "AbstractMethodError";
    }

    constructor(s: String) {
        super(s);
        this.name = "AbstractMethodError";
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
        this.name = "AbstractMethodError";
    }
}
