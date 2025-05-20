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

export final class Runtime {

    /**
     * Returns a hash code for the Object.
     *
     * @param o to calculate hash code from
     *
     * @returns hash code
     */
    public static native getHashCode(o: Object): int;

     /**
      * Checks if two items are equal.
      *
      * @param o1 first object to compare
      *
      * @param o2 second object to compare
      *
      * @returns true if items are equal
      */
    public static equals(o1: Any, o2: Any): boolean {
        if (Runtime.isSameReference(o1, null)) {
            return Runtime.isSameReference(o2, null)
        }
        if (Runtime.isSameReference(o1, undefined)) {
            return Runtime.isSameReference(o2, undefined)
        }
        if (Runtime.isSameReference(o2, null) || Runtime.isSameReference(o2, undefined)) {
            return false;
        }

        return o1 == o2
    }

    /**
     * Checks if two items are equal 'by value'
     *
     * @param o1 first object to compare
     *
     * @param o2 second object to compare
     *
     * @returns true if items are equal 'by value'
     */
    public static sameValue(o1: Any, o2: Any): boolean {
        if (Runtime.isSameReference(o1, null)) {
            return Runtime.isSameReference(o2, null)
        }
        if (Runtime.isSameReference(o1, undefined)) {
            return Runtime.isSameReference(o2, undefined)
        }
        if (Runtime.isSameReference(o2, null) || Runtime.isSameReference(o2, undefined)) {
            return false;
        }

        if (o1 instanceof Number && o2 instanceof Number) {
            return Runtime.sameNumberValue(o1 as Number, o2 as Number)
        } else if (o1 instanceof Float && o2 instanceof Float) {
            return Runtime.sameFloatValue(o1 as Float, o2 as Float)
        }

        return o1 == o2
    }

    private static sameNumberValue(n1: Number, n2: Number): boolean {
        const n1val = n1.valueOf()
        const n2val = n2.valueOf()
        if (n1val == 0 || n2val == 0) {
            return Double.bitCastToLong(n1val) == Double.bitCastToLong(n2val);
        }
        if (isNaN(n1val) != isNaN(n2val)) {
            return false;
        } else if (isNaN(n1val)) {
            return true;
        } else {
            return n1val == n2val
        }
    }

    private static sameFloatValue(f1: Float, f2: Float): boolean {
        const f1val = f1.unboxed()
        const f2val = f2.unboxed()

        if (f1val == 0 || f2val == 0) {
            return Float.bitCastToInt(f1val) == Float.bitCastToInt(f2val)
        }

        if (isNaN(f1val) != isNaN(f2val)) {
            return false
        } else if (isNaN(f1val)) {
            return true
        } else {
            return f1val == f2val
        }
    }

    public static sameValueZero(o1: Any, o2: Any): boolean {
        if (Runtime.sameValue(o1, o2)) {
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
     * Check if it is a little-endian platform
     *
     * @returns true - little-endian, false - otherwise
     */
    public native static isLittleEndianPlatform(): boolean;

    /**
     * Checks if two references point to a same object/value.
     *
     * @param o1 first object to compare
     *
     * @param o2 second object to compare
     *
     * @returns true if Objects are equal
     */
    public static native isSameReference(o1: Any, o2: Any): boolean;

    /**
     * Internal entrypoint to create exception for failed type cast
     *
     * @param src source value
     *
     * @param dstStr target type string
     *
     * @returns exception object to throw
     */
    static native failedTypeCastException(src: Any, dstStr: string): ClassCastError

    native static __getTypeInfo(o: Any): Class|undefined;

    native static allocSameTypeArray<T extends FixedArray<Any>>(type: Class, length: int): T;

    static addFrontElementsStub(original: FixedArray<Any>, front: FixedArray<Any>): FixedArray<Any> {
        let res = Runtime.allocSameTypeArray<FixedArray<Any>>(Class.of(original), original.length + front.length)
        for (let i: int = 0; i < front.length; ++i) {
            res[i] = front[i]
        }
        for (let i: int = 0; i < original.length; ++i) {
            res[i + front.length] = original[i]
        }
        return res
    }

    static removeFrontElementsStub(original: FixedArray<Any>, count: int): FixedArray<Any> {
        let res = Runtime.allocSameTypeArray<FixedArray<Any>>(Class.of(original), original.length - count)
        for (let i: int = count; i < original.length; ++i) {
            res[i - count] = original[i]
        }
        return res
    }

    static restArgumentsUnderapplicationStub(rest: FixedArray<Any>, required: int): FixedArray<Any> {
        if (rest.length >= required) {
            return rest
        }
        let newRest : FixedArray<Any> = new Any[required]
        for (let i: int = 0; i < rest.length; ++i) {
            newRest[i] = rest[i]
        }
        for (let i: int = rest.length; i < required; ++i) {
            newRest[i] = undefined
        }
        return newRest
    }
}

// Deprecated. Narrow the value to type T or to default (object | null | undefined).
export function __narrowAny<T = (object | null | undefined)>(v: Any): T {
    return v as T
}

export function __runtimeEquals(o1: Any, o2: Any): boolean {
    return Runtime.equals(o1, o2)
}

export function __runtimeSameValueZero(o1: Any, o2: Any): boolean {
    return Runtime.sameValueZero(o1, o2)
}

export function __runtimeSameValue(o1: Any, o2: Any): boolean {
    return Runtime.sameValue(o1, o2)
}

export function __runtimeIsSameReference(o1: Any, o2: Any): boolean {
    return Runtime.isSameReference(o1, o2)
}
