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
 * @class Represents an error that occurs when memory allocation fails
 */
export class OutOfMemoryError extends Exception {
    constructor() {
        super();
    }

    constructor(s: String) {
        super(s);
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
    }
}

/**
 * @class Represents an error that occurs when no such method is found in the class
 */
export class NoSuchMethodError extends Exception {
    constructor() {
        super();
    }

    constructor(s: String) {
        super(s);
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
    }
}

/**
 * @class Represents an error that occurs when the available memory is not sufficient to create the activation frame
 */
export class StackOverflowError extends Exception {
    constructor() {
        super();
    }

    constructor(s: String) {
        super(s);
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
    }
}

/**
 * @class Represents an error that occurs when attempting to instantiate abstract class or an interface
 */
export class InstantiationError extends Exception {
    constructor() {
        super();
    }

    constructor(s: String) {
        super(s);
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
    }
}

/**
 * @class Represents an error that occurs when an internal error occured
 */
export class InternalError extends Exception {
    constructor() {
        super();
    }

    constructor(s: String) {
        super(s);
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
    }
}

/**
 * @class Represents an error that occurs when no class definition has been found
 */
export class NoClassDefFoundError extends Exception {
    constructor() {
        super();
    }

    constructor(s: String) {
        super(s);
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
    }
}

/**
 * @class Represents an error that occurs when no such field is found in the class
 */
export class NoSuchFieldError extends Exception {
    constructor() {
        super();
    }

    constructor(s: String) {
        super(s);
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
    }
}

/**
 * The AbstractMethodError object represents an error that occures when abstract method get called.
 */
export class AbstractMethodError extends Exception {
    constructor() {
        super();
    }

    constructor(s: String) {
        super(s);
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
    }
}

/**
 * Represents exception that is thrown in case of null pointer dereference
 */
export final class NullPointerException extends Exception {
    constructor() {
        super();
    }

    constructor(s: String) {
        super(s);
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
    }
}

// TODO(orlovskiymaxim): what cause?
/**
 * Represents exception that is thrown in case of ?
 */
export final class NoDataException extends Exception {
    constructor() {
        super();
    }

    constructor(s: String) {
        super(s);
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
    }
}

/**
 * Represents exception that is thrown when provided argument have value outside the allowable range
 */
export final class ArgumentOutOfRangeException extends Exception {
    constructor() {
        super();
    }

    constructor(s: String) {
        super(s);
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
    }
}

/**
 * Represents exception that is thrown when provided collection index is out of bounds
 */
export class IndexOutOfBoundsException extends Exception {
    constructor() {
        super();
    }

    constructor(s: String) {
        super(s);
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
    }
}

/**
 * Represents exception that is thrown when a method has been invoked at an illegal or inappropriate time.
 */
export class IllegalStateException extends Exception {
    constructor() {
        super();
    }

    constructor(s: String) {
        super(s);
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
    }
}

/**
 * Represents exception that is thrown when provided array index is out of bounds
 */
export final class ArrayIndexOutOfBoundsException extends IndexOutOfBoundsException {
    constructor() {
        super();
    }

    constructor(s: String) {
        super(s);
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
    }
}

/**
 * Represents exception that is thrown when provided string index is out of bounds
 */
export final class StringIndexOutOfBoundsException extends IndexOutOfBoundsException {
    constructor() {
        super();
    }

    constructor(s: String) {
        super(s);
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
    }
}

/**
 * Represents exception that is thrown when illegal arithmetic operation is occured (e.g. division by zero)
 */
export final class ArithmeticException extends Exception {
    constructor() {
        super();
    }

    constructor(s: String) {
        super(s);
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
    }
}

/**
 * Represents exception that is thrown in case of illegal class casting
 */
export final class ClassCastException extends Exception {
    constructor() {
        super();
    }

    constructor(s: String) {
        super(s);
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
    }
}

// TODO(orlovskiymaxim): where exactly not found?
/**
 * Represents exception that is thrown when class is not found ?
 */
export final class ClassNotFoundException extends Exception {
    constructor() {
        super();
    }

    constructor(s: String) {
        super(s);
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
    }
}

/**
 * Represents exception that is thrown when the requested operation is not supported.
 */
export final class UnsupportedOperationException extends Exception {
    constructor() {
        super();
    }

    constructor(s: String) {
        super(s);
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
    }
}

/**
 * @class Represents exception that is thrown when attempting to store an object of different type in array of type-erased objects
 */
export final class ArrayStoreException extends Exception {
    constructor() {
        super();
    }

    constructor(s: String) {
        super(s);
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
    }
}

/**
 * Represents exception that is thrown when negative array size is supplied
 */
export final class NegativeArraySizeException extends Exception {
    constructor() {
        super();
    }

    constructor(s: String) {
        super(s);
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
    }
}

/**
 * @class Represents exception that is thrown when attempting to wait, notify or notifyAll on object, that hasn't been synchronised
 */
export final class IllegalMonitorStateException extends Exception {
    constructor() {
        super();
    }

    constructor(s: String) {
        super(s);
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
    }
}

/**
 * @class Represents exception that is thrown when
 * a method has been passed an illegal argument
 */
export final class IllegalArgumentException extends Exception {
    constructor() {
        super();
    }

    constructor(s: String) {
        super(s);
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
    }
}

export class ExceptionInInitializerError extends Exception {
    constructor() {
        super();
    }

    constructor(s: String) {
        super(s);
    }

    constructor(s: String, cause: Object) {
        super(s, cause);
    }
}
