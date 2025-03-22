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
            return this.sameNumberValue(o1 as Number, o2 as Number)
        } else if (o1 instanceof Float && o2 instanceof Float) {
            return this.sameFloatValue(o1 as Float, o2 as Float)
        }

        return o1 == o2
    }
    
    private sameNumberValue(n1: Number, n2: Number): boolean {
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

    private sameFloatValue(f1: Float, f2: Float): boolean {
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
    static failedTypeCastException(src: NullishType, dstStr: string): ClassCastError {
        let srcStr = src === null ? "null" :
            (src === undefined ? "undefined" :
                Class.of(src).getName());
        return new ClassCastError(srcStr + " cannot be cast to " + dstStr);
    }

    native static __getTypeInfo(o: NullishType): Class|undefined;

    native static allocSameTypeArray<T extends NullishType[]>(type: Class, length: int): T;

    static addFrontElementsStub(original: __T[], front: __T[]): __T[] {
        let res = Runtime.allocSameTypeArray<__T[]>(Class.of(original), original.length + front.length)
        for (let i: int = 0; i < front.length; ++i) {
            res[i] = front[i]
        }
        for (let i: int = 0; i < original.length; ++i) {
            res[i + front.length] = original[i]
        }
        return res
    }

    static removeFrontElementsStub(original: __T[], count: int): __T[] {
        let res = Runtime.allocSameTypeArray<__T[]>(Class.of(original), original.length - count)
        for (let i: int = count; i < original.length; ++i) {
            res[i - count] = original[i]
        }
        return res
    }

    static restArgumentsUnderapplicationStub(rest: __T[], required: int): __T[] {
        if (rest.length >= required) {
            return rest
        }
        let newRest = new __T[required]
        for (let i: int = 0; i < rest.length; ++i) {
            newRest[i] = rest[i]
        }
        for (let i: int = rest.length; i < required; ++i) {
            newRest[i] = undefined
        }
        return newRest
    }
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
