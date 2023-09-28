/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
 * BigInt class stub
 */
export class BigInt {
    constructor() {}
    constructor(d: int) {}
    constructor(d: long) {}
    constructor(d: double) {}
    constructor(d: string) {}
    constructor(d: boolean) {}
    constructor(d: BigInt) {}

    internal getLong(): long { return 0 }

    public static asIntN(bits: int, d: BigInt): BigInt {
        throw new Error("BigInt.asIntN: not implemented")
    }

    public static asUintN(bits: int, d: BigInt): BigInt {
        throw new Error("BigInt.asUintN: not implemented")
    }

    /**
     * Returns a locale string representing the specified array and its elements.
     *
     * @param locales
     *
     * @param options
     *
     * @returns string representation
     */
    public toLocaleString(locales: Object, options: Object): string {
        throw new Error("BigInt.toLocaleString: not implemented")
    }

    /**
     * Returns a locale string representing the specified array and its elements.
     *
     * @param options
     *
     * @returns string representation
     */
    public toLocaleString(locales: Object): string {
        return this.toLocaleString(new Object(), new Object())
    }

    /**
     * Returns a locale string representing the specified array and its elements.
     *
     * @returns string representation
     */
    public toLocaleString(): string {
        return this.toLocaleString(new Object(), new Object())
    }

    public override toString(): string {
        throw new Error("BigInt.toString: not implemented")
    }

    public valueOf(): BigInt {
        // TODO: return this? Unclear description in js stdlib
        throw new Error("BigInt.valueOf: not implemented")
    }
}
