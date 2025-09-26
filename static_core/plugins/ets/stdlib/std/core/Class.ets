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

/**
 * Unstable API, represents internal runtime type descriptor
 */
export final class Class {
    private name: string | undefined
    private superClass: Class | undefined
    private flags: int
    private typeMetaData: long
    private overloadMap: long

    private constructor() { throw new Error("Class constructor called") }
    private native getNameInternal(): string;

    public getName(): string {
        return this.name ?? this.getNameInternal()
    }

    /**
     * Gets the super class of a class
     *
     * @returns The superclass of a class, or `undefined` if this is either Object or interface class
     */
    public getSuper(): Class | undefined {
        return this.superClass
    }

    public native final isSubtypeOf(other: Class): boolean

    public native getLinker(): RuntimeLinker

    public native isNamespace(): boolean;

    public getNamespace(): Class | undefined {
        throw new Error("not implemented")
    }

    public native getFixedArrayComponentType(): Class | undefined

    public native getInterfaces(): FixedArray<Class>

    /**
     * Invoke class initializer once if class is not initialized
     */
    public native initialize(): void

    private static native ofObject(obj: Object): Class

    private static native ofNull(): Class

    public static ofAny(obj: Any): Class | undefined {
        if (obj instanceof ESValue || obj === undefined) {
            return undefined
        } else if (obj instanceof Object) {
            return Class.of(obj as Object)
        }
        return Class.ofNull()
    }

    public static of(obj: Object | null): Class {
        if (obj instanceof Object) {
            return Class.ofObject(obj)
        }
        return Class.ofNull()
    }

    /**
     * Get current class
     */
    public static native current(): Class

    /**
     * Get class of caller
     */
    public static native ofCaller(): Class | undefined

    public native createInstance(): Object

    /**
     * Get internal string descriptor
     */
    public native getDescriptor(): string

    public native isEnum(): boolean

    public native isInterface(): boolean

    public native isFixedArray(): boolean

    /**
     * Check if class is union
     *
     * @returns true if class is union
     */
    public native isUnion(): boolean

    /**
     * Gets only public instance methods of a class
     *
     * @returns Immutable array of instance {@link reflect.InstanceMethod} objects
     */
    public getInstanceMethods(): FixedArray<reflect.InstanceMethod> {
        return this.getInstanceMethodsInternal(true)
    }

    /**
     * Looks up an public instance method by name
     *
     * @param name - The method name to search for
     *
     * @returns The {@link reflect.InstanceMethod} if found, `undefined` otherwise
     */
    public getInstanceMethodByName(name: string): reflect.InstanceMethod | undefined {
        return this.getInstanceMethodByNameInternal(name, true)
    }

    /**
     * Looks up an public instance field by name
     *
     * @param name - The field name to search for
     *
     * @returns The {@link reflect.InstanceField} if found, `undefined` otherwise
     */
    public getInstanceFieldByName(name: string): reflect.InstanceField | undefined {
        return this.getInstanceFieldByNameInternal(name, true)
    }

    /**
     * Looks up an public static method by name
     *
     * @param name - The method name to search for
     * @param signature - The method signature to search for (optional). If omitted, first match is returned
     *
     * @returns The {@link reflect.StaticMethod} if found, `undefined` otherwise
     */
    public getStaticMethodByName(name: string, signature?: FixedArray<Class>): reflect.StaticMethod | undefined {
        let cls: Class | undefined = this
        while (cls != undefined) {
            let methodFoundByName = cls.getDirectStaticMethodByNameInternal(name, undefined, true)
            if (methodFoundByName != undefined) {
                if (signature == undefined) {
                    return methodFoundByName
                }
                return cls.getDirectStaticMethodByNameInternal(name, signature, true)
            }
            cls = cls.getSuper()
        }
        return undefined
    }

    /**
     * Gets only public instance fields of a class
     *
     * @returns Immutable array of instance {@link reflect.InstanceField} objects
     */
    public getInstanceFields(): FixedArray<reflect.InstanceField> {
        if (this.isInterface()) {
            return []
        }
        return this.getInstanceFieldsInternal(true)
    }

    /**
     * Gets only public static methods of a class
     *
     * @returns Immutable array of static {@link reflect.StaticMethod} objects
     */
    public getStaticMethods(): FixedArray<reflect.StaticMethod> {
        return this.getStaticMethodsInternal(true)
    }

    /**
     * Gets only public static fields of a class
     *
     * @returns Immutable array of static {@link reflect.StaticField} objects
     */
    public getStaticFields(): FixedArray<reflect.StaticField> {
        if (this.isInterface()) {
            return []
        }
        return this.getStaticFieldsInternal(true)
    }

    /**
     * Looks up a public static field by name
     *
     * @param name - The field name to search for
     *
     * @returns The {@link reflect.StaticField} if found, `undefined` otherwise
     */
    public getStaticFieldByName(name: string): reflect.StaticField | undefined {
        return this.getStaticFieldByNameInternal(name, true)
    }

    /**
     * Gets all public constructors of a class
     *
     * @returns Immutable array of constructor {@link reflect.Constructor} objects
     */
    public getConstructors(): FixedArray<reflect.Constructor> {
        return this.getConstructorsInternal(true)
    }

    /**
     * Gets all constituent types of a union class
     *
     * @returns Immutable array of Class objects or undefined if class is not union
     */
    public getUnionConstituentTypes(): FixedArray<Class> | undefined {
        if (!this.isUnion()) {
            return undefined
        }
        return this.getUnionConstituentTypesInternal()
    }

    /**
     * Check the class is final
     */
    public native isFinal(): boolean

    /**
     * Check the class is abstract
     */ 
    public native isAbstract(): boolean

    private native getInstanceMethodsInternal(publicOnly: boolean): FixedArray<reflect.InstanceMethod>
    private native getInstanceMethodByNameInternal(name: string, publicOnly: boolean): reflect.InstanceMethod | undefined
    private native getInstanceFieldByNameInternal(name: string, publicOnly: boolean): reflect.InstanceField | undefined
    private native getDirectStaticMethodByNameInternal(name: string, signature: FixedArray<Class> | undefined, publicOnly: boolean): reflect.StaticMethod | undefined
    private native getInstanceFieldsInternal(publicOnly: boolean): FixedArray<reflect.InstanceField>
    private native getStaticMethodsInternal(publicOnly: boolean): FixedArray<reflect.StaticMethod>
    private native getStaticFieldsInternal(publicOnly: boolean): FixedArray<reflect.StaticField>
    private native getStaticFieldByNameInternal(name: string, publicOnly: boolean): reflect.StaticField | undefined
    private native getConstructorsInternal(publicOnly: boolean): FixedArray<reflect.Constructor>
    private native getUnionConstituentTypesInternal(): FixedArray<Class>
}
