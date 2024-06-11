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
 * Represents exception that is thrown when class is not found
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
 * Represents exception that is thrown when verifier detects errors in class
 */
export final class VerifyError extends Exception {
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
 * Represents exception that is thrown when linker detects errors in class
 */
export final class LinkageError extends Exception {
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
 * Represents exception that is thrown when runtime detects incompatible changes in class
 */
export final class IncompatibleClassChangeError extends Exception {
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