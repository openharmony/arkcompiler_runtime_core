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
 * Represents an error that occurs when assertion fails.
 */

export final class AssertionError extends Error {
    readonly tmp_console = new Console();

    constructor(message: String) {
        super(message);
        this.tmp_console.print(message);
        this.tmp_console.println();
        // exit
    }

    constructor(message: String, cause: Object) {
        super(message, cause);
        this.tmp_console.print(message);
        this.tmp_console.println();
        // exit
    }

    private exit() : void {}
}

/**
 * Represents an error that occurs when division by zero is performed.
 */
export final class DivideByZeroError extends Error {}

/**
 * Represents an error that occurs when null pointer is dereferenced.
 */
export final class NullPointerError extends Error {}

/**
 * Represents an error that occurs when exception is thrown and not catched
 */
export final class UncatchedExceptionError extends Error {}

/**
 * @class Represents an error that occurs when JSON is malformed
 */
export final class SyntaxError extends Error {
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
 * @class Represents an error that occurs when JSONValue can not be assigned to a type
 */
export final class JSONTypeError extends Error {
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
 * @class Represents an error that occurs when range is invalid
 */
export final class RangeError extends Error {
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
 * @class Represents an error that occurs when array is oging to be indexed out of its bounds
 */
export final class ArrayIndexOutOfBoundsError extends Error {
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