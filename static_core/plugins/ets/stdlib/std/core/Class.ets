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
 * @file Defines the Class for ArkTS
 * @kit ArkTS
 */

package std.core;

/**
 * Class used to describe runtime types
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class Class {
    private name: string | undefined
    private superClass: Class | undefined
    private linker: RuntimeLinker | undefined
    private flags: int
    private typeMetaData: long
    private methodsNum: int

    /**
     * Store Class of boolean primitive type
     * @type { Class }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly PRIMITIVE_BOOLEAN: Class = Class.of(null)

    /**
     * Store Class of byte primitive type
     * @type { Class }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly PRIMITIVE_BYTE: Class = Class.of(null)

    /**
     * Store Class of byte primitive type
     * @type { Class }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly PRIMITIVE_CHAR: Class = Class.of(null)

    /**
     * Store Class of short primitive type
     * @type { Class }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly PRIMITIVE_SHORT: Class = Class.of(null)

    /**
     * Store Class of int primitive type
     * @type { Class }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly PRIMITIVE_INT: Class = Class.of(null)

    /**
     * Store Class of long primitive type
     * @type { Class }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly PRIMITIVE_LONG: Class = Class.of(null)

    /**
     * Store Class of float primitive type
     * @type { Class }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly PRIMITIVE_FLOAT: Class = Class.of(null)

    /**
     * Store Class of double primitive type
     * @type { Class }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly PRIMITIVE_DOUBLE: Class = Class.of(null)

    /**
     * Store Class of number primitive type (same as double)
     * @type { Class }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly PRIMITIVE_NUMBER: Class = Class.of(null)

    /**
     * Store Class of void type
     * @type { Class }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly PRIMITIVE_VOID: Class = Class.of(null)

    static {
        initializePrimitivesInClass()

        Class.PRIMITIVE_BOOLEAN.name = 'u1'
        Class.PRIMITIVE_BYTE.name = 'i8'
        Class.PRIMITIVE_SHORT.name = 'i16'
        Class.PRIMITIVE_CHAR.name = 'u16'
        Class.PRIMITIVE_INT.name = 'i32'
        Class.PRIMITIVE_LONG.name = 'i64'
        Class.PRIMITIVE_FLOAT.name = 'f32'
        Class.PRIMITIVE_DOUBLE.name = 'f64'
        Class.PRIMITIVE_NUMBER.name = 'f64'
        Class.PRIMITIVE_VOID.name = 'void'
    }

    private constructor() { throw new Error("Class constructor called") }
    private native getNameInternal(): string
    private native getLinkerInternal(): RuntimeLinker | undefined

    /**
     * Get the name of a class in assembly
     *
     * @returns { string } return class name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getName(): string {
        if (this.name === undefined) {
            this.name = this.getNameInternal()
        }
        return this.name!
    }

    /**
     * Get the super class of a class
     *
     * @returns { Class | undefined } return the super class
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getSuper(): Class | undefined {
        return this.superClass
    }

    /**
     * Check if this class is a subtype of another class.
     *
     * @param { Class } other The class to check against.
     * @returns { boolean } Returns true if this class is a subtype of the other class, false otherwise.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native final isSubtypeOf(other: Class): boolean

    /**
     * Get the runtime linker associated with this class.
     *
     * @returns { RuntimeLinker } the runtime linker for this class
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getLinker(): RuntimeLinker {
        if (this.linker == undefined) {
            const linker = this.getLinkerInternal()
            this.linker = linker ?? getBootRuntimeLinker()
        }
        return this.linker!
    }

    /** 
     * Check if this class represents a namespace.
     * 
     * @returns { boolean } true if this class is a namespace, false otherwise.
     * @syscap SystemCapability.Utils.Lang 
     * @since 24 
     */
    public native isNamespace(): boolean;

    public getNamespace(): Class | undefined {
        throw new Error("not implemented")
    }
    /** 
     * Get the component type of this class if it is a fixed array.
     * 
     * @returns { Class | undefined } the component type of the fixed array, or undefined if this class is not a fixed array.
     * @syscap SystemCapability.Utils.Lang 
     * @since 24 
     */ 
    public native getFixedArrayComponentType(): Class | undefined

    /** 
     * Get the interfaces implemented by this class.
     * 
     * @returns { FixedArray<Class> } a fixed array of interfaces implemented by the class.
     * @syscap SystemCapability.Utils.Lang 
     * @since 24 
     */ 
    public native getInterfaces(): FixedArray<Class>

    /** 
     * Invoke class initializer once if class is not initialized.
     * 
     * @syscap SystemCapability.Utils.Lang 
     * @since 24 
     */
    public native initialize(): void

    private static native ofObject(obj: Object): Class

    /** 
     * Get the class of an object of any type.
     * 
     * @param { Any } obj The object to get the class from.
     * @returns { Class | undefined } the class of the object, or undefined if the object is not a class instance.
     * @static 
     * @syscap SystemCapability.Utils.Lang 
     * @since 24 
     */ 
    public static ofAny(obj: Any): Class | undefined {
        if (obj instanceof Object) {
            return Class.of(obj as Object)
        }
        if (obj === null) {
            return Class.from<null>()
        }
        return undefined
    }

    /** 
     * Get the class of an object.
     * 
     * @param { Object | null } obj The object to get the class from.
     * @returns { Class } the class of the object.
     * @static 
     * @syscap SystemCapability.Utils.Lang 
     * @since 24 
     */ 
    public static of(obj: Object | null): Class {
        if (obj instanceof Object) {
            return Class.ofObject(obj)
        }
        return Class.from<null>()
    }

    public static from<T>(): Class {
        //should never be executed
        throw new AssertionError("Class.from call will be casted by frontend")
    }
    /** 
     * Get current class.
     * 
     * @returns { Class } the current class.
     * @static 
     * @syscap SystemCapability.Utils.Lang 
     * @since 24 
     */ 
    public static native current(): Class

    /** 
     * Get class of caller.
     * 
     * @returns { Class | undefined } the class of the caller, or undefined if there is no caller managed frame.
     * @static 
     * @syscap SystemCapability.Utils.Lang 
     * @since 24 
     */
    public static native ofCaller(): Class | undefined
    /**
     * Create a new instance of this class and invokes its public parameterless constructor.
     *
     * @returns { Object } a new instance of this class.
     * @throws { Error } if the class is not instantiable or does not have a public default constructor.
     * @throws { Error } if class was not previously initialized and initialization fails.
     * @throws { Error } if constructor throws.
     * @throws { OutOfMemoryError } if there is not enough memory to allocate the object.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native createInstance(): Object

    public native getDescriptor(): string
    /**
     * Check if this class is an enum.
     *
     * @returns { boolean } true if this class is an enum, false otherwise.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native isEnum(): boolean
    /**
     * Check if this class is an interface.
     *
     * @returns { boolean } true if this class is an interface, false otherwise.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native isInterface(): boolean
    /**
     * Check if this class is a fixed array.
     *
     * @returns { boolean } true if this class is a fixed array, false otherwise.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native isFixedArray(): boolean

    /**
     * Check if this class is a union type.
     *
     * @returns { boolean } true if this class is a union type, false otherwise.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native isUnion(): boolean

    /** 
     * Get instance methods (including default methods from implemented interfaces) declared in the class.
     * 
     * @returns { FixedArray<reflect.InstanceMethod> } a fixed array of instance methods.
     * @syscap SystemCapability.Utils.Lang 
     * @since 24 
     */
    public getInstanceMethods(): FixedArray<reflect.InstanceMethod> {
        return this.getInstanceMethodsInternal(true)
    }

    /** 
     * Look up a instance method (including default methods from implemented interfaces) among the methods 
     * declared in the class.
     * 
     * @param { string } name The method name to search for.
     * @param { FixedArray<Class> | undefined } [ signature ] Array of parameter classes defining the method signature. 
     * @returns { reflect.InstanceMethod | undefined } the found instance method, or undefined if not found.
     * @throws { Error } When signature is omitted and multiple overloads exist
     * @syscap SystemCapability.Utils.Lang 
     * @since 24 
     */
    public getInstanceMethod(name: string, signature?: FixedArray<Class>): reflect.InstanceMethod | undefined {
        return this.getInstanceMethodInternal(name, signature, true)
    }

    /** 
     * Get static methods declared in the class.
     * 
     * @returns { FixedArray<reflect.StaticMethod> } a fixed array of static methods.
     * @syscap SystemCapability.Utils.Lang 
     * @since 24 
     */
    public getStaticMethods(): FixedArray<reflect.StaticMethod> {
        if (this.isInterface()) {
            return []
        }
        return this.getStaticMethodsInternal(true)
    }

    /** 
     * Look up a static method among the methods declared in the class.
     * 
     * @param { string } name The method name to search for.
     * @param { FixedArray<Class> | undefined } [ signature ] Array of parameter classes defining the method signature. 
     * @returns { reflect.StaticMethod | undefined } the found static method, or undefined if not found.
     * @throws {Error} When signature is omitted and multiple overloads exist
     * @syscap SystemCapability.Utils.Lang 
     * @since 24 
     */
    public getStaticMethod(name: string, signature?: FixedArray<Class>): reflect.StaticMethod | undefined {
        let methods = this.getStaticMethods()
        if (methods.length == 0) {
            return undefined
        }
        if (signature == undefined) {
            return Class.findStaticMethodByNameUnchecked(methods, name)
        }
        return Class.findStaticMethodBySignatureUnchecked(methods, name, signature)
    }

    /**
     * Get instance fields declared in the class.
     *
     * @returns { FixedArray<reflect.InstanceField> } a fixed array of instance fields.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getInstanceFields(): FixedArray<reflect.InstanceField> {
        if (this.isInterface()) {
            return []
        }
        return this.getInstanceFieldsInternal(true)
    }

    /**
     * Look up a instance field by name among the fields declared in this class.
     *
     * @param { string } name The field name to search for.
     * @returns { reflect.InstanceField | undefined } the found instance field, or undefined if not found.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getInstanceField(name: string): reflect.InstanceField | undefined {
        return this.getInstanceFieldByNameInternal(name, true)
    }

    /**
     * Get static fields declared in this class.
     *
     * @returns { FixedArray<reflect.StaticField> } a fixed array of static fields.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getStaticFields(): FixedArray<reflect.StaticField> {
        if (this.isInterface()) {
            return []
        }
        return this.getStaticFieldsInternal(true)
    }

    /**
     * Look up a static field by name among the fields declared in this class.
     *
     * @param { string } name The field name to search for.
     * @returns { reflect.StaticField | undefined } the found static field, or undefined if not found.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getStaticField(name: string): reflect.StaticField | undefined {
        return this.getStaticFieldByNameInternal(name, true)
    }

    /**
     * Get all constructors of a class.
     *
     * @returns { FixedArray<reflect.Constructor> } a fixed array of constructors.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getConstructors(): FixedArray<reflect.Constructor> {
        return this.getConstructorsInternal(true)
    }

    /**
     * Get all constituent types of a union class.
     *
     * @returns { FixedArray<Class> | undefined } a fixed array of constituent types, 
     * or undefined if this is not a union class.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getUnionConstituentTypes(): FixedArray<Class> | undefined {
        if (!this.isUnion()) {
            return undefined
        }
        return this.getUnionConstituentTypesInternal()
    }

    /**
     * Check the class is final
     *
     * @returns { boolean } true if the class is fianl
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native isFinal(): boolean

    /**
     * Check the class is abstract
     *
     * @returns { boolean } true if the class is abstract
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native isAbstract(): boolean

    /**
     * Check if class is primitive type
     *
     * @returns { boolean } true if the class corresponds to a primitive type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native isPrimitive(): boolean

    // NOTE(kurnevichstanislav): squash two implementations into one after #31016 is fixed.
    private static findStaticMethodByNameUnchecked(
        methods: FixedArray<reflect.StaticMethod>,
        name: string
    ): reflect.StaticMethod | undefined {
        let candidate: reflect.StaticMethod | undefined = undefined

        for (let method of methods) {
            if (method.getName() == name) {
                if (candidate != undefined) {
                    throw new Error(`Found two methods with the same name '${name}'. Please specify signature.`)
                }
                candidate = method
            }
        }
        return candidate
    }

    // NOTE(kurnevichstanislav): squash two implementations into one after #31016 is fixed.
    private static findStaticMethodBySignatureUnchecked(
        methods: FixedArray<reflect.StaticMethod>,
        name: string,
        signature: FixedArray<Class>
    ): reflect.StaticMethod | undefined {
        for (let method of methods) {
            if (method.getName() == name && Class.compareSignatureStatic(method, signature)) {
                return method
            }
        }
        return undefined
    }

    // NOTE(kurnevichstanislav): squash two implementations into one after #31016 is fixed.
    private static compareSignatureStatic(method: reflect.StaticMethod, signature: FixedArray<Class>): boolean {
        if (method.getParametersNum() != signature.length) {
            return false
        }
        for (let idx = 0; idx < signature.length; ++idx) {
            if (method.getParameterType(idx) != signature[idx]) {
                return false
            }
        }
        return true
    }

    private native getInstanceMethodsInternal(publicOnly: boolean): FixedArray<reflect.InstanceMethod>
    private native getInstanceMethodInternal(name: string, signature: FixedArray<Class> | undefined, publicOnly: boolean): reflect.InstanceMethod | undefined
    private native getInstanceFieldByNameInternal(name: string, publicOnly: boolean): reflect.InstanceField | undefined
    private native getInstanceFieldsInternal(publicOnly: boolean): FixedArray<reflect.InstanceField>
    private native getStaticMethodsInternal(publicOnly: boolean): FixedArray<reflect.StaticMethod>
    private native getStaticFieldsInternal(publicOnly: boolean): FixedArray<reflect.StaticField>
    private native getStaticFieldByNameInternal(name: string, publicOnly: boolean): reflect.StaticField | undefined
    private native getConstructorsInternal(publicOnly: boolean): FixedArray<reflect.Constructor>
    private native getUnionConstituentTypesInternal(): FixedArray<Class>
}

native function initializePrimitivesInClass(): void
