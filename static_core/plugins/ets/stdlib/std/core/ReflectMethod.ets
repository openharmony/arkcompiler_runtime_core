/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
/**
 * @file Defines the ReflectMethod for ArkTS
 * @kit ArkTS
 */


package std.core;

/**
 * Core namespace for reflection-related functionality, mainly used to obtain class and method information.
 *
 * @namespace reflect
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export namespace reflect {

/**
 * Represents a method of a class or interface.
 */
class Method {
    protected name: string | undefined
    protected ownerType: Class
    protected methodPtr: long
    protected attributes: int
    protected accessMod: byte

    /**
     * Protected native constructor of the Method class.
     *
     */
    protected native constructor()

    /**
     * Checks if the method is a native method.
     *
     * @returns { boolean } Returns true if it is a native method, otherwise returns false.
     * 
     */
    public isNative(): boolean {
        return (this.attributes & Attributes.NATIVE) != 0
    }

    /**
     * Checks if the method is a public method.
     *
     * @returns { boolean } Returns true if it is a public method, otherwise returns false.
     * 
     */
    public isPublic(): boolean {
        return (this.accessMod == AccessModifier.PUBLIC)
    }

    /**
     * Checks if the method is a private method.
     *
     * @returns { boolean } Returns true if it is a private method, otherwise returns false.
     * 
     */
    public isPrivate(): boolean {
        return (this.accessMod == AccessModifier.PRIVATE)
    }

    /**
     * Checks if the method is a protected method.
     *
     * @returns { boolean } Returns true if it is a protected method, otherwise returns false.
     * 
     */
    public isProtected(): boolean {
        return (this.accessMod == AccessModifier.PROTECTED)
    }

    /**
     * Gets the name of the method.
     *
     * @returns { string } Returns the name string of the method.
     * 
     */
    public getName(): string {
        return this.name ?? Method.getNameInternal(this.methodPtr)
    }

    /**
     * Gets the class or interface that declares this method.
     *
     * @returns { Class } Returns the Class object that declares this method.
     * 
     */
    public getOwner(): Class {
        return this.ownerType
    }

    /**
     * Gets the return type of the method.
     *
     * @returns { Class } Returns the Class object representing the method's return type.
     *
     */
    public getReturnType(): Class {
        return Method.getReturnTypeImpl(this.methodPtr)!
    }

    /**
     * Gets the types of all parameters.
     *
     * @returns { FixedArray<Class> } Returns a FixedArray containing all parameter types.
     *
     */
    public getParametersTypes(): FixedArray<Class> {
        return Method.getParametersTypesImpl(this.methodPtr)
    }

    /**
     * Gets the type of the parameter at the specified index by index.
     *
     * @param { int } idx Index position of the parameter.
     * @returns { Class | undefined } Returns the Class object of the parameter type at the specified index,
     * returns undefined if the index is invalid.
     *
     */
    public getParameterType(idx: int): Class | undefined {
        return Method.getParameterTypeByIdxImpl(this.methodPtr, idx)
    }

    /**
     * Gets the number of method parameters.
     *
     * @returns { int } Returns the number of method parameters.
     * 
     */
    public getParametersNum(): int {
        return Method.getParametersNumImpl(this.methodPtr)
    }

    /**
     * Invokes the underlying method represented by this method instance.
     *
     * @param { Object | undefined } thisObj The this object used when calling the method,
     * should be undefined for static methods.
     * @param { FixedArray<Any> } args The array of arguments passed when calling the method.
     * @returns { Any } Returns the result of method execution.
     * 
     */
    protected invoke(thisObj: Object | undefined, args?: FixedArray<Any>): Any {
        this.ownerType.initialize()
        return this.invokeInternal(thisObj, args)
    }

    private static native getNameInternal(methodPtr: long): string
    private static native getReturnTypeImpl(methodPtr: long): Class | undefined
    private static native getParametersTypesImpl(methodPtr: long): FixedArray<Class>
    private static native getParameterTypeByIdxImpl(methodPtr: long, idx: int): Class | undefined
    private static native getParametersNumImpl(methodPtr: long): int
    private native invokeInternal(thisObj: Object | undefined, args?: FixedArray<Any>): Any
}

} // namespace reflect
