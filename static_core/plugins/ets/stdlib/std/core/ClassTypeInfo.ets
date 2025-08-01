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

final class ClassTypeInfoNativeIface {
    public static native getInstanceMethods(type: Class, onlyPublic: boolean): FixedArray<Method>
    public static native getInstanceMethodByName(type: Class, name: string, onlyPublic: boolean): Method | undefined

    public static native getBase(type: Class): Class | undefined
}

/**
 * Provides detailed reflection information about class or interface types
 */
export final class ClassTypeInfo extends TypeInfo {
    private constructor(type: Class) {
        super(type)
    }

    /**
     * Checks whether a class is declared as final
     *
     * @returns `true` if the class is final, `false` otherwise
     */
    public isFinal(): boolean {
        throw new Error("Not implemeted yet")
    }

    /**
     * Checks whether a type represents an interface
     *
     * @returns `true` if this is an interface type, `false` otherwise
     */
    public isInterface(): boolean {
        throw new Error("Not implemeted yet")
    }

    /**
     * Gets the base class of a class
     *
     * @returns The superclass of a class, or `undefined` if this is the Object class
     */
    public getBase(): Class | undefined {
        return ClassTypeInfoNativeIface.getBase(this.type)
    }

    /**
     * Gets the module where a class is declared
     *
     * @returns The containing {@link Module} instance
     */
    public getModule(): Module {
        throw new Error("Not implemeted yet")
    }

    /**
     * Gets all public fields (both static and instance) of a class
     *
     * @returns Immutable array of all {@link Field} objects
     */
    public getFields(): FixedArray<Field> {
        throw new Error("Not implemeted yet")
    }

    /**
     * Gets only public instance fields of a class
     *
     * @returns Immutable array of instance {@link Field} objects
     */
    public getInstanceFields(): FixedArray<Field> {
        throw new Error("Not implemeted yet")
    }

    /**
     * Gets only public static fields of a class
     *
     * @returns Immutable array of static {@link Field} objects
     */
    public getStaticFields(): FixedArray<Field> {
        throw new Error("Not implemeted yet")
    }

    /**
     * Looks up an public instance field by name
     *
     * @param name - The field name to search for
     *
     * @returns The {@link Field} if found, `undefined` otherwise
     */
    public getInstanceFieldByName(name: string): Field | undefined {
        throw new Error("Not implemeted yet")
    }

    /**
     * Looks up a public static field by name
     *
     * @param name - The field name to search for
     *
     * @returns The {@link Field} if found, `undefined` otherwise
     */
    public getStaticFieldByName(name: string): Field | undefined {
        throw new Error("Not implemeted yet")
    }

    /**
     * Gets all public methods (both static and instance) of a class
     *
     * @returns Immutable array of all {@link Method} objects
     */
    public getMethods(): FixedArray<Method> {
        throw new Error("Not implemeted yet")
    }

    /**
     * Gets only public instance methods of a class
     *
     * @returns Immutable array of instance {@link Method} objects
     */
    public getInstanceMethods(): FixedArray<Method> {
        return ClassTypeInfoNativeIface.getInstanceMethods(this.type, true);
    }

    /**
     * Gets only public static methods of a class
     *
     * @returns Immutable array of static {@link Method} objects
     */
    public getStaticMethods(): FixedArray<Method> {
        throw new Error("Not implemeted yet")
    }

    /**
     * Looks up an public instance method by name
     *
     * @param name - The method name to search for
     *
     * @returns The {@link Method} if found, `undefined` otherwise
     */
    public getInstanceMethodByName(name: string): Method | undefined {
        return ClassTypeInfoNativeIface.getInstanceMethodByName(this.type, name, true)
    }

    /**
     * Looks up a public static method by name
     *
     * @param name - The method name to search for
     *
     * @returns The {@link Method} if found, `undefined` otherwise
     */
    public getStaticMethodByName(name: string): Method | undefined {
        throw new Error("Not implemeted yet")
    }

    /**
     * Gets all public constructors of a class
     *
     * @returns Immutable array of constructor {@link Method} objects
     */
    public getConstructors(): FixedArray<Method> {
        throw new Error("Not implemeted yet")
    }

    /**
     * Gets all interfaces implemented by a class
     *
     * @returns Immutable array of interface {@link Method} objects
     */
    public getInterfaces(): FixedArray<Method> {
        throw new Error("Not implemeted yet")
    }
}

} // namespace reflect
