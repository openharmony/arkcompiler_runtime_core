/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

export namespace reflect {

export /* sealed */ abstract class TypeInfo {
    private type_: Class

    protected constructor(type: Class) {
        throw new Error("Attempt of creation of TypeInfo through constructor")
    }

    protected get type(): Class {
        return this.type_;
    }

    /**
     * Creates a TypeInfo instance for the given class
     *
     * @param type instance of {@link Type} class
     *
     * @returns A new {@link TypeInfo} instance describing the given type
     */
    public static of(type: Class): TypeInfo {
        return TypeInfo.ofImpl(type);
    }

    /**
     * Checks whether the current type is a subtype of the specified type
     *
     * @param other - The potential supertype to check against (TypeInfo instance)
     *
     * @returns `true` if this type is a subtype of `other`, `false` otherwise
     */
    public isSubtypeOf(other: TypeInfo): boolean {
        throw new Error("Not implemented yet.")
    }

    /**
     * Returns name of type
     *
     * @returns name of type
     */
    public getName(): string {
        return TypeInfo.getNameImpl(this.type_)
    }

    /**
     * Gets the underlying Type instance that this TypeInfo describes
     *
     * @returns The contained {@link Type} instance that this TypeInfo wraps
     */
    public getUnderlyingType(): Class {
        return this.type_
    }

    private static native getNameImpl(klass: Class): string
    private static native ofImpl(klass: Class): TypeInfo
}

} // namespace reflect
