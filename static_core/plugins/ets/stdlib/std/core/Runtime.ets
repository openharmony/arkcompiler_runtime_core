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

export final class Runtime {
    internal constructor() {}

    /**
     * Returns a hash code for the Object.
     *
     * @param o to calculate hash code from
     *
     * @returns hash code
     */
    public native getHashCode(o: Object): int;

    /**
     * Checks if two items are equal.
     *
     * @param o1 first object to compare
     *
     * @param o2 second object to compare
     *
     * @returns true if items are equal
     */
    public equals(o1: NullishType, o2: NullishType): boolean {
        if (this.isSameReference(o1, null)) {
            return this.isSameReference(o2, null)
        }
        if (this.isSameReference(o1, undefined)) {
            return this.isSameReference(o2, undefined)
        }
        if (this.isSameReference(o2, null) || this.isSameReference(o2, undefined)) {
            return false;
        }
        return o1!.equals(o2!);
    }

    public sameValue(o1: NullishType, o2: NullishType): boolean {
        if (this.isSameReference(o1, null)) {
            return this.isSameReference(o2, null)
        }
        if (this.isSameReference(o1, undefined)) {
            return this.isSameReference(o2, undefined)
        }
        if (this.isSameReference(o2, null) || this.isSameReference(o2, undefined)) {
            return false;
        }
        if (o1 instanceof Number && o2 instanceof Number) {
            const n1 = (o1 as Number).valueOf()
            const n2 = (o2 as Number).valueOf()
            const pos0 = Double.bitCastToLong(+0);
            const neg0 = Double.bitCastToLong(-0);
            if (n1 == 0 || n2 == 0) {
                return Double.bitCastToLong(n1) == Double.bitCastToLong(n2);
            }
            if (isNaN(n1) != isNaN(n2)) {
                return false;
            }
            if (isNaN(n1)) {
                return true;
            }
            if (n1 == n2) {
                return true;
            }
        }
        return o1!.equals(o2!);
    }

    public sameValueZero(o1: NullishType, o2: NullishType): boolean {
        if (this.sameValue(o1, o2)) {
            return true;
        }
        if (o1 instanceof Number && o2 instanceof Number) {
            const n1 = (o1 as Number).valueOf()
            const n2 = (o2 as Number).valueOf()
            return n1 == 0 && n2 == 0;
        }
        return false;
    }

    /**
     * Checks if two references point to a same object/value.
     *
     * @param o1 first object to compare
     *
     * @param o2 second object to compare
     *
     * @returns true if Objects are equal
     */
    public native isSameReference(o1: NullishType, o2: NullishType): boolean;

    /**
     * Internal entrypoint to create exception for failed type cast
     * 
     * @param src source value
     * 
     * @param dstStr target type string
     * 
     * @returns exception object to throw
     */
    internal static failedTypeCastException(src: NullishType, dstStr: string): ClassCastException {
        let srcStr = src === null ? "null" :
            (src === undefined ? "undefined" :
                TypeAPIGetTypeDescriptor(src));
        return new ClassCastException(srcStr + " cannot be cast to " + dstStr);
    }

    /**
     * Return a string with the name of the type kind of the object
     *
     * @param o an object
     *
     * @returns a string with the name of the type kind of the object
     */
    internal native static typeOf(o: Object): String;
}

export const runtime = new Runtime();

export function __runtimeEquals(o1: NullishType, o2: NullishType): boolean {
    return runtime.equals(o1, o2)
}

export function __runtimeSameValueZero(o1: NullishType, o2: NullishType): boolean {
    return runtime.sameValueZero(o1, o2)
}

export function __runtimeSameValue(o1: NullishType, o2: NullishType): boolean {
    return runtime.sameValue(o1, o2)
}

export function __runtimeIsSameReference(o1: NullishType, o2: NullishType): boolean {
    return runtime.isSameReference(o1, o2)
}

/**
 * Loads library from specified path.
 *
 * @param path to library to load
 */
export native function loadLibrary(path: String): void;
