/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS of ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/**
 * @file Defines the TypeCreator for ArkTS
 * @kit ArkTS
 */

package std.core;

/**
 * Internal class defining allowed attribute combinations for different type creator elements
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
class AllowedAttributes {
    /**
     * Allowed attributes for fields
     * @static
     * @readonly
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly FIELD = Attributes.STATIC | Attributes.READONLY
    /**
     * Allowed attributes for methods
     * @static
     * @readonly
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly METHOD = Attributes.STATIC | Attributes.FINAL | Attributes.ABSTRACT | Attributes.CONSTRUCTOR | Attributes.NATIVE | Attributes.ASYNC | Attributes.GETTER | Attributes.SETTER
    /**
     * Allowed attributes for lambda types
     * @static
     * @readonly
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly LAMBDA = Attributes.ASYNC
    /**
     * Allowed attributes for classes
     * @static
     * @readonly
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly CLASS = Attributes.FINAL
    /**
     * Allowed attributes for parameters
     * @static
     * @readonly
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly PARAMETER = Attributes.REST | Attributes.OPTIONAL
}

/**
 * Error that is thrown when exception in create-api occurs
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export class TypeAPICreateError extends Error {
    public constructor(msg: string) {
        super(msg)
    }
}

/**
 * Type of function that collects all reachable types for creator context
 * @typedef { functon } CollectTypesCB
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
type CollectTypesCB = (tc: TypeOrCreator) => void

/**
 * Base for all type creators, supports creation of {@link Type}
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export abstract class TypeCreator {
    protected typ?: Type // not undefined if already created
    protected ctxItemPtr: long = 0 // resides in ctx, no need to free

    /**
     * Forces creation of `this` creator and creators it depends on
     *
     * @returns { Type }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public create(): Type {
        if (this.typ == undefined) {
            const ctx = new TypeCreatorCtx()
            const targetLinker = this.getTargetLinker()
            try {
                ctx.create(this, targetLinker)
            } finally {
                ctx.deinit()
            }
        }
        return this.typ!
    }

    protected abstract getTargetLinker(): RuntimeLinker

    /**
     * Traverses all types on which this `TypeCreator` depends
     * @param { CollectTypesCB } collector callback to be called with every type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public abstract collectTypes(collector: CollectTypesCB): void

    /**
     * Must declare up to the point where future type descriptor is determinated
     * @param { TypeCreatorCtx } ctx current context
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public abstract declare(ctx: TypeCreatorCtx): void

    /**
     * Defines rest of the body, called after {@link declare}
     * @param { TypeCreatorCtx } ctx current context
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public abstract define(ctx: TypeCreatorCtx): void

    /**
     * Makes class sealed in stdlib
     *
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor() {}

    /**
     * Functions that verifies that this can be modified
     */
    protected checkNotCreated(): void {
        if (this.typ != undefined) throw new TypeAPICreateError("type already created, no modifications allowed")
    }

    /**
     * getTypeDescriptor
     *
     * @returns { RuntimeTypeDescriptor }
     * @throws { AssertionError } - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getTypeDescriptor(): RuntimeTypeDescriptor {
        if (this.typ != undefined) {
            return this.typ!.getTypeDesc()
        }
        if (this.ctxItemPtr === 0) {
            throw new AssertionError("this.ctxItemPtr is 0")
        }
        return TypeCreatorCtx.getTypeDescFromPointer(this.ctxItemPtr)
    }

    /**
     * setTyp
     *
     * @param { Type } typ_ The type to be set
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public setTyp(typ_: Type) {
        this.typ = typ_
    }

    /**
     * getTyp
     *
     * @returns { Type | undefined }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getTyp(): Type | undefined {
        return this.typ
    }
}

/**
 * semantically same as Type | TypeCreator, but provides unified way to get descriptor
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export abstract class TypeOrCreator {
    /**
     * Returns runtime type descriptor for this type.
     * @returns { RuntimeTypeDescriptor } type descriptor or future type descriptor
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    abstract getTypeDescriptor(): RuntimeTypeDescriptor

    /**
     * from
     *
     * @param { Type } typ None
     * @returns { TypeOrCreator } created {@link TypeOrCreator_Type}
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    static from(typ: Type): TypeOrCreator {
        return new TypeOrCreator_Type(typ)
    }

    /**
     * from
     *
     * @param { TypeCreator } typ None
     * @returns { TypeOrCreator } created {@link TypeOrCreator_Creator} or TypeOrCreator_Type if
     *     it was already created.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    static from(typ: TypeCreator): TypeOrCreator {
        if (typ.getTyp() != undefined) {
            return new TypeOrCreator_Type(typ.getTyp()!)
        }
        return new TypeOrCreator_Creator(typ)
    }
}

/**
 * TypeOrCreator wrapper for Type instances
 * @extends TypeOrCreator
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
final class TypeOrCreator_Type extends TypeOrCreator {
    private typ: Type

    /**
     * Overrides getTypeDescriptor to return type descriptor from Type
     *
     * @returns { RuntimeTypeDescriptor } type descriptor
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    override getTypeDescriptor(): RuntimeTypeDescriptor {
        return this.typ.getTypeDesc()
    }

    /**
     * Constructor
     *
     * @param { Type } typ Type instance to wrap
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(typ: Type) {
        this.typ = typ
    }
}

/**
 * TypeOrCreator wrapper for TypeCreator instances
 * @extends TypeOrCreator
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
final class TypeOrCreator_Creator extends TypeOrCreator {
    /**
     * TypeCreator instance being wrapped
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    typ: TypeCreator

    /**
     * Overrides getTypeDescriptor to return type descriptor from TypeCreator
     *
     * @returns { RuntimeTypeDescriptor } type descriptor
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    override getTypeDescriptor(): RuntimeTypeDescriptor {
        return this.typ.getTypeDescriptor()
    }

    /**
     * Constructor
     *
     * @param { TypeCreator } typ TypeCreator instance to wrap
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(typ: TypeCreator) {
        this.typ = typ
    }
}

export final class TypeCreatorCtx {
    /**
     * Pointer to native context
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public ctxPtr: long
    /** all objects that were passed to cpp code as their ids (indexes in this `Array`) */
    private objects: Array<Object> = new Array<Object>();

    private static native createCtx(): long
    private static native destroyCtx(ctxPtr: long): void

    /**
     * Commits all created types into runtime.
     *
     * @param { long } ctxPtr pointer to native context.
     * @param { FixedArray<Object> } objects array of saved objects passed to static constructor.
     * @param { RuntimeLinker } targetLinker in which context code will be loaded.
     * @returns { string | undefined } string in case of occurred errors.
     * @throws errors happened on class initialization.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    private static native commit(ctxPtr: long, objects: FixedArray<Object>, targetLinker: RuntimeLinker): string | undefined

    private static native getError(ctxPtr: long): string | undefined

    /**
     * getObjectsArrayForCCtor
     *
     * @param { long } ctxPtr used to identify the construction context.
     * @returns { FixedArray<Object> }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native getObjectsArrayForCCtor(ctxPtr: long): FixedArray<Object>

    /**
     * getTypeDescFromPointer
     *
     * @param { long } ctxItemPtr Pointer to a context item containing type information.
     * @returns { RuntimeTypeDescriptor }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native getTypeDescFromPointer(ctxItemPtr: long): RuntimeTypeDescriptor

    /**
     * classCreate
     *
     * @param { long } ctxPtr The context pointer for memory allocation and class registration.
     * @param { string } name The name of the class to be created.
     * @param { int } attrs The attributes/flags for the class.
     * @returns { long }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native classCreate(ctxPtr: long, name: string, attrs: int): long

    /**
     * classSetBase
     *
     * @param { long } classPtr Pointer to the class structure to modify.
     * @param { RuntimeTypeDescriptor } base Type descriptor of the class.
     * @returns { string | undefined }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native classSetBase(classPtr: long, base: RuntimeTypeDescriptor): string | undefined

    /**
     * classAddIface
     *
     * @param { long } classPtr Pointer to the class structure to modify.
     * @param { RuntimeTypeDescriptor } iface Type descriptor of the interface to implement.
     * @returns { string | undefined }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native classAddIface(classPtr: long, iface: RuntimeTypeDescriptor): string | undefined

    /**
     * classAddField
     *
     * @param { long } classPtr Pointer to target class.
     * @param { string } name Field name.
     * @param { RuntimeTypeDescriptor } field Field type descriptor.
     * @param { int } attrs Field attributes.
     * @param { int } access Access modifiers.
     * @returns { string | undefined }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native classAddField(classPtr: long, name: string, field: RuntimeTypeDescriptor, attrs: int, access: int): string | undefined

    /**
     * interfaceCreate
     *
     * @param { long } ctxPtr The context pointer for memory allocation and interface registration.
     * @param { string } name The name of the interface to be created.
     * @returns { long }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native interfaceCreate(ctxPtr: long, name: string): long

    /**
     * interfaceAddBase
     *
     * @param { long } ifacePtr Pointer to the interface structure to modify.
     * @param { RuntimeTypeDescriptor } base Type descriptor of the interface.
     * @returns { string | undefined }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native interfaceAddBase(ifacePtr: long, base: RuntimeTypeDescriptor): string | undefined

    /**
     * lambdaTypeCreate
     *
     * @param { long } ctxPtr The context pointer for memory allocation and type registration.
     * @param { int } attrs The attributes for the lambda type
     * @returns { long }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native lambdaTypeCreate(ctxPtr: long, attrs: int): long

    /**
     * lambdaTypeAddParam
     *
     * @param { long } ftPtr Pointer to the lambda function type structure.
     * @param { RuntimeTypeDescriptor } param Type descriptor of the parameter to add.
     * @param { int } attrs Parameter attributes
     * @returns { string | undefined }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native lambdaTypeAddParam(ftPtr: long, param: RuntimeTypeDescriptor, attrs: int): string | undefined

    /**
     * lambdaTypeAddResult
     *
     * @param { long } ftPtr Pointer to the lambda function type structure.
     * @param { RuntimeTypeDescriptor } result Type descriptor of the return type.
     * @returns { string | undefined }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native lambdaTypeAddResult(ftPtr: long, result: RuntimeTypeDescriptor): string | undefined

    /**
     * lambdaTypeAdd
     *
     * @param { long } ftPtr Lambda type pointer to finalize.
     * @returns { string | undefined }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native lambdaTypeAdd(ftPtr: long): string | undefined

    /**
     * methodCreate
     *
     * @param { long } ctxItemPtr Pointer to parent class/interface.
     * @param { string } name Method name.
     * @param { int } attrs Method attributes.
     * @returns { long }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native methodCreate(ctxItemPtr: long, name: string, attrs: int): long

    /**
     * methodAddAccessMod
     *
     * @param { long } methPtr Pointer to target method.
     * @param { int } access Access modifier value.
     * @returns { string | undefined }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native methodAddAccessMod(methPtr: long, access: int): string | undefined

    /**
     * methodAddParam
     *
     * @param { long } methPtr Pointer to target method.
     * @param { RuntimeTypeDescriptor } td Parameter type descriptor.
     * @param { string } name Parameter name.
     * @param { int } attrs Parameter attributes.
     * @returns { string | undefined }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native methodAddParam(methPtr: long, td: RuntimeTypeDescriptor, name: string, attrs: int): string | undefined

    /**
     * methodAddResult
     *
     * @param { long } methPtr Pointer to target method.
     * @param { RuntimeTypeDescriptor } result Return type descriptor.
     * @returns { string | undefined }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native methodAddResult(methPtr: long, result: RuntimeTypeDescriptor): string | undefined

    /**
     * methodAddBodyFromMethod
     *
     * @param { long } methPtr Pointer to target method.
     * @param { MethodType } methodType Source method for implementation.
     * @returns { string | undefined }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native methodAddBodyFromMethod(methPtr: long, methodType: MethodType): string | undefined

    /**
     * methodAddBodyFromLambda
     *
     * @param { long } methPtr Pointer to target method.
     * @param { int } id Lambda identifier.
     * @param { LambdaType } lambdaType Lambda type descriptor.
     * @returns { string | undefined }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native methodAddBodyFromLambda(methPtr: long, id: int, lambdaType: LambdaType): string | undefined

    /**
     * methodAddBodyFromErasedLambda
     *
     * @param { long } methPtr Method pointer to receive implementation.
     * @param { int } id Runtime identifier of erased lambda.
     * @returns { string | undefined }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native methodAddBodyFromErasedLambda(methPtr: long, id: int): string | undefined

    /**
     * methodAddBodyDefault
     *
     * @param { long } methPtr Pointer to the method structure to modify.
     * @returns { string | undefined }
     * @throws { AssertionError } - Input parameter error.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native methodAddBodyDefault(methPtr: long): string | undefined

    /**
     * methodAdd
     *
     * @param { long } methPtr Pointer to method to finalize.
     * @returns { string | undefined }
     * @throws { AssertionError } - Input parameter error.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native methodAdd(methPtr: long): string | undefined

    /**
     * checks that context is errorneous used after long-returning types (that can't throw an exception explicitly)
     *
     * @throws { AssertionError } - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public checkError(): void {
        this.checkError(TypeCreatorCtx.getError(this.ctxPtr))
    }

    /**
     * Checks for error if pointer is invalid
     *
     * @param { long } ptr pointer to one of items allocated in the context
     * @returns { long }
     * @throws { AssertionError } - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public checkCtxItemPtr(ptr: long): long {
        if (ptr == 0) {
            this.checkError()
        }
        return ptr
    }

    /**
     * Cleans up offheap memory for this context. Must be called.
     *
     * @throws { AssertionError } - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public deinit(): void {
        if (this.ctxPtr === 0) {
            throw new AssertionError("cannot call deinit when ctxPtr is null")
        }
        TypeCreatorCtx.destroyCtx(this.ctxPtr)
        this.ctxPtr = 0
    }

    /**
     * default constructor
     *
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor() {
        this.ctxPtr = TypeCreatorCtx.createCtx()
    }

    /**
     * Runs entire type creation routine, including all types collection
     *
     * @param { TypeCreator } first creator with must be created
     * @param { RuntimeLinker } targetLinker in which type creation must be committed
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public create(first: TypeCreator, targetLinker: RuntimeLinker): void {
        const all = new Set<TypeCreator>()
        const visited = new Set<TypeCreator>()

        const order = new Array<TypeCreator>()

        // bfs traverser
        const push: CollectTypesCB = (tc: TypeOrCreator): void => {
            if (tc instanceof TypeOrCreator_Type) {
                return
            }
            const typeCreator = (tc as TypeOrCreator_Creator).typ
            if (typeCreator.getTyp() != undefined) {
                return
            }
            if (visited.has(typeCreator)) {
                return
            }
            order.push(typeCreator)
            all.add(typeCreator)
        }

        push(new TypeOrCreator_Creator(first))

        // bfs
        let was = true
        while (was) {
            was = false
            const asArr = new Array<TypeCreator>()

            all.forEach((creator: TypeCreator): void => {
                asArr.push(creator)
            })
            for (let i = 0; i < asArr.length; i++) {
                const el = asArr.$_get(i)
                if (!visited.has(el)) {
                    was = true
                    visited.add(el)
                    el.collectTypes(push)
                }
            }
        }

        // reversing list
        for (let i = 0; i < order.length / 2; i++) {
            const j = order.length - 1 - i
            const tmp = order.$_get(i)
            order.$_set(i, order.$_get(j))
            order.$_set(j, tmp)
        }

        // forward declare all entities
        for (let i = 0; i < order.length; i++) {
            order.$_get(i).declare(this)
        }

        // defines all entities (i.e. provides bodies)
        for (let i = 0; i < order.length; i++) {
            order.$_get(i).define(this)
        }

        this.checkError()

        let objsArray: FixedArray<Object> = new FixedArray<Object>(this.objects.length)
        for (let i = 0; i < this.objects.length; i++) {
            objsArray[i] = this.objects.$_get(i) as Object
        }
        this.checkError(TypeCreatorCtx.commit(this.ctxPtr, objsArray, targetLinker))

        // assignes actual runtime types to all creators
        for (let i = 0; i < order.length; i++) {
            const tc = order.$_get(i)
            let resolved = Type.resolve(tc.getTypeDescriptor(), targetLinker) 
            if (resolved == undefined) {
                throw new TypeAPICreateError("Can not resolve Type")
            }
            tc.setTyp(resolved!)
        }
    }

    /**
     * Saves object that is used in generated code. Example: lambdas of body methods.
     *
     * @param { Object } o Object
     * @returns { int }
     * @throws { TypeAPICreateError } - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public registerObject(o: Object): int {
        const r = this.objects.length
        this.objects.push(o)
        return r.toInt()
    }

    /**
     * Checks if ctx function returned an error
     *
     * @param { string | undefined } arg Error string or undefined.
     * @throws { TypeAPICreateError } TypeAPICreateError if error was found
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public checkError(arg: string | undefined): void {
        if (arg != undefined) {
            throw new TypeAPICreateError(arg as string)
        }
    }
}

/**
 * Creator for {@link ClassType}
 * @extends TypeCreator
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class ClassTypeCreator extends TypeCreator {
    private base: TypeOrCreator = TypeOrCreator.from(ObjectType)
    private name: string
    private ifaces: Array<TypeOrCreator> = new Array<TypeOrCreator>()
    private fields: Array<FieldCreator> = new Array<FieldCreator>()
    private methods: Array<MethodCreator> = new Array<MethodCreator>()
    private attrs: HasAttributes
    private targetLinker: RuntimeLinker

    /**
     * constructor
     *
     * @param { string } name fully qualified name of class type, can include only [A-Za-z0-9_.]
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(name: string) {
        super()
        this.attrs = new HasAttributes(new FreezableCreator(this), AllowedAttributes.CLASS)
        this.name = name
        this.targetLinker = getNearestNonBootRuntimeLinker()!
    }

    /**
     * Forces creation of `this` creator and creators it depends on
     *
     * @returns { ClassType }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override create(): ClassType {
        return super.create() as ClassType
    }

    /**
     * Sets base class (super, that in ets follows `extends` keyword)
     *
     * @param { ClassType } base new base
     * @returns { ClassTypeCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addBaseType(base: ClassType): ClassTypeCreator {
        this.checkNotCreated()
        this.base = TypeOrCreator.from(base)
        return this;
    }

    /**
     * Sets base class (super, that in ets follows `extends` keyword)
     *
     * @param { ClassTypeCreator } base new base
     * @returns { ClassTypeCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addBaseType(base: ClassTypeCreator): ClassTypeCreator {
        this.checkNotCreated()
        this.base = TypeOrCreator.from(base)
        return this;
    }

    /**
     * Adds implemented interface (in ets it follows `implements` keyword)
     *
     * @param { InterfaceType } iface interface to implement
     * @returns { ClassTypeCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addInterface(iface: InterfaceType): ClassTypeCreator {
        this.ifaces.push(TypeOrCreator.from(iface))
        return this;
    }

    /**
     * Adds implemented interface (in ets it follows `implements` keyword)
     *
     * @param { InterfaceTypeCreator } iface interface to implement
     * @returns { ClassTypeCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addInterface(iface: InterfaceTypeCreator): ClassTypeCreator {
        this.ifaces.push(TypeOrCreator.from(iface))
        return this;
    }

    /**
     * Adds field
     *
     * @param { FieldCreator } fld field to add
     * @returns { ClassTypeCreator }
     * @throws { AssertionError } - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addField(fld: FieldCreator): ClassTypeCreator {
        fld.getFrozen().freeze()
        this.fields.push(fld)
        return this;
    }

    /**
     * collectTypes
     *
     * @param { CollectTypesCB } collector Function to process each type.
     * @throws { AssertionError } - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override collectTypes(collector: CollectTypesCB): void {
        collector(this.base)
        for (let i = 0; i < this.ifaces.length; i++) {
            collector(this.ifaces.$_get(i))
        }
        for (let i = 0; i < this.fields.length; i++) {
            collector(this.fields.$_get(i).typ)
        }
        for (let i = 0; i < this.methods.length; i++) {
            const m = this.methods.$_get(i)
            collector(m.result)
            for (let i = 0; i < m.params.length; i++) {
                collector(m.params.$_get(i).typ)
            }
        }
    }

    /**
     * Declares the class type in the context
     *
     * @param { TypeCreatorCtx } ctx current context
     * @throws { AssertionError } - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override declare(ctx: TypeCreatorCtx): void {
        if (this.ctxItemPtr !== 0) {
            throw new AssertionError("this.ctxItemPtr is not 0")
        }
        this.ctxItemPtr = ctx.checkCtxItemPtr(TypeCreatorCtx.classCreate(ctx.ctxPtr, this.name, this.attrs.get()))
    }

    /**
     * Defines the class type with base class, interfaces, fields, and methods
     *
     * @param { TypeCreatorCtx } ctx current context
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override define(ctx: TypeCreatorCtx): void {
        const classPtr = this.ctxItemPtr
        ctx.checkError(TypeCreatorCtx.classSetBase(classPtr, this.base.getTypeDescriptor()))
        for (let i = 0; i < this.ifaces.length; i++) {
            ctx.checkError(TypeCreatorCtx.classAddIface(classPtr, this.ifaces.$_get(i).getTypeDescriptor()))
        }
        for (let i = 0; i < this.fields.length; i++) {
            const f = this.fields.$_get(i);
            ctx.checkError(TypeCreatorCtx.classAddField(classPtr, f.name, f.typ.getTypeDescriptor(), f.attrs.get(), f.getAccessMod()))
        }
        for (let i = 0; i < this.methods.length; i++) {
            const m = this.methods.$_get(i)
            m.addToCtxItemPtr(ctx, classPtr)
        }
    }

    /**
     * Gets the target runtime linker
     *
     * @returns { RuntimeLinker } target linker
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    protected override getTargetLinker(): RuntimeLinker {
        return this.targetLinker
    }

    /**
     * Adds method
     *
     * @param { MethodCreator } mb method to add
     * @returns { ClassTypeCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addMethod(mb: MethodCreator): ClassTypeCreator {
        mb.frozen.freeze()
        this.methods.push(mb)
        return this;
    }

    /**
     * Sets all provided attributes to `true`
     *
     * @param { int } attrs attribute to set
     * @returns { ClassTypeCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addAttributes(attrs: int): ClassTypeCreator {
        this.attrs.add(attrs)
        return this;
    }

    /**
     * Adds attribute with same name
     *
     * @returns { ClassTypeCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addFinal(): ClassTypeCreator {
        this.attrs.add(Attributes.FINAL, true)
        return this;
    }
}

/**
 * Creator for {@link InterfaceType}
 * @extends TypeCreator
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class InterfaceTypeCreator extends TypeCreator {
    private name: string
    private bases: Array<TypeOrCreator> = new Array<TypeOrCreator>()
    private methods: Array<MethodCreator> = new Array<MethodCreator>()
    private targetLinker: RuntimeLinker

    /**
     * constructor
     *
     * @param { string } name name of interface
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(name: string) {
        super()
        this.name = name
        this.targetLinker = getNearestNonBootRuntimeLinker()!
    }

    /**
     * Forces creation of `this` creator and creators it depends on
     *
     * @returns { InterfaceType }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override create(): InterfaceType {
        return super.create() as InterfaceType
    }

    /**
     * Adds implemented interface (in ets it follows `extends` keyword)
     *
     * @param { InterfaceType } base Interface type to implement.
     * @returns { InterfaceTypeCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addInterface(base: InterfaceType): InterfaceTypeCreator {
        this.checkNotCreated()
        this.bases.push(TypeOrCreator.from(base))
        return this;
    }

    /**
     * Adds implemented interface (in ets it follows `extends` keyword)
     *
     * @param { InterfaceTypeCreator } base Interface creator providing implementation.
     * @returns { InterfaceTypeCreator }
     * @throws { TypeAPICreateError } - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addInterface(base: InterfaceTypeCreator): InterfaceTypeCreator {
        this.checkNotCreated()
        this.bases.push(TypeOrCreator.from(base))
        return this;
    }

    /**
     * Adds getter and optionally setter to `this` interface
     *
     * @param { FieldCreator } fld property description
     * @returns { InterfaceTypeCreator }
     * @throws { TypeAPICreateError } - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addField(fld: FieldCreator): InterfaceTypeCreator {
        const getter = new MethodCreator(fld.name).addResult(fld.typ).addGetter()
        this.addMethod(getter)
        if (!fld.attrs.has(Attributes.READONLY)) {
            const setter = new MethodCreator(fld.name).addParameter(new ParameterCreator(fld.typ)).addSetter()
            this.addMethod(setter)
        }
        return this;
    }

    /**
     * Adds method to `this` interface
     *
     * @param { MethodCreator } mc method to add
     * @returns { InterfaceTypeCreator }
     * @throws { TypeAPICreateError } - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addMethod(mc: MethodCreator): InterfaceTypeCreator {
        if ((mc.attrs.has(Attributes.STATIC))) {
            throw new TypeAPICreateError("interface can't have static method " + mc)
        }
        mc.addAbstract()
        mc.addAccessModifier(AccessModifier.PUBLIC)
        mc.frozen.freeze()
        this.methods.push(mc)
        return this;
    }

    /**
     * Collects all types on which this creator depends
     *
     * @param { CollectTypesCB } collector callback to be called with every type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override collectTypes(collector: CollectTypesCB): void {
        for (let i = 0; i < this.bases.length; i++) {
            collector(this.bases.$_get(i))
        }
        for (let i = 0; i < this.methods.length; i++) {
            const m = this.methods.$_get(i)
            collector(m.result)
            for (let i = 0; i < m.params.length; i++) {
                collector(m.params.$_get(i).typ)
            }
        }
    }

    /**
     * Defines the interface type with base interfaces and methods
     *
     * @param { TypeCreatorCtx } ctx current context
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override define(ctx: TypeCreatorCtx): void {
        const classPtr = this.ctxItemPtr
        for (let i = 0; i < this.bases.length; i++) {
            ctx.checkError(TypeCreatorCtx.interfaceAddBase(classPtr, this.bases.$_get(i).getTypeDescriptor()))
        }
        for (let i = 0; i < this.methods.length; i++) {
            const m = this.methods.$_get(i)
            m.addToCtxItemPtr(ctx, classPtr)
        }
    }

    /**
     * Declares the interface type in the context
     *
     * @param { TypeCreatorCtx } ctx current context
     * @throws { AssertionError } - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override declare(ctx: TypeCreatorCtx): void {
        if (this.ctxItemPtr !== 0) {
            throw new AssertionError("this.ctxItemPtr is not 0")
        }
        this.ctxItemPtr = ctx.checkCtxItemPtr(TypeCreatorCtx.interfaceCreate(ctx.ctxPtr, this.name))
    }

    /**
     * Gets the target runtime linker
     *
     * @returns { RuntimeLinker } target linker
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    protected override getTargetLinker(): RuntimeLinker {
        return this.targetLinker
    }
}

/**
 * Creator for {@link LambdaType}
 * @extends TypeCreator
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class LambdaTypeCreator extends TypeCreator {
    /**
     * Parameters of the lambda type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public params: Array<ParameterCreator> = new Array<ParameterCreator>();
    /**
     * Return type of the lambda
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public result: TypeOrCreator = TypeOrCreator.from(VoidType.REF);
    private attrs: HasAttributes
    private targetLinker: RuntimeLinker

    /**
     * default constructor
     *
     * @throws { TypeAPICreateError } - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor() {
        this.attrs = new HasAttributes(new FreezableCreator(this), AllowedAttributes.LAMBDA)
        this.targetLinker = getNearestNonBootRuntimeLinker()!
    }

    /**
     * Forces creation of `this` creator and creators it depends on
     *
     * @returns { LambdaType }
     * @throws { TypeAPICreateError } - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override create(): LambdaType {
        return super.create() as LambdaType
    }

    /**
     * Appends parameter to `this` type
     *
     * @param { ParameterCreator } param parameter to add
     * @returns { LambdaTypeCreator }
     * @throws { TypeAPICreateError } - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addParameter(param: ParameterCreator): LambdaTypeCreator {
        this.checkNotCreated()
        param.frozen.freeze()
        if (param.name != undefined && param.name != '' + this.params.length) {
            throw new TypeAPICreateError("can't have such lambda parameter name")
        }
        this.params.push(param)
        return this;
    }

    /**
     * Sets result
     *
     * @param { Type } typ type of result
     * @returns { LambdaTypeCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addResult(typ: Type): LambdaTypeCreator {
        return this.addResult(TypeOrCreator.from(typ));
    }

    /**
     * Sets result
     *
     * @param { TypeCreator } typ type of result
     * @returns { LambdaTypeCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addResult(typ: TypeCreator): LambdaTypeCreator {
        return this.addResult(TypeOrCreator.from(typ));
    }

    /**
     * Sets result
     *
     * @param { TypeOrCreator } typ type of result
     * @returns { LambdaTypeCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addResult(typ: TypeOrCreator): LambdaTypeCreator {
        this.checkNotCreated()
        this.result = typ
        return this;
    }

    /**
     * Sets all provided attributes to `true`
     *
     * @param { int } attrs attribute to set
     * @returns { LambdaTypeCreator }
     * @throws { AssertionError } - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addAttributes(attrs: int): LambdaTypeCreator {
        this.attrs.add(attrs)
        return this;
    }

    /**
     * Adds attribute with same name
     *
     * @returns { LambdaTypeCreator }
     * @throws { AssertionError } - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addAsync(): LambdaTypeCreator {
        this.attrs.add(Attributes.ASYNC, true)
        return this;
    }

    /**
     * Collects all types on which this creator depends
     *
     * @param { CollectTypesCB } collector callback to be called with every type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override collectTypes(collector: CollectTypesCB): void {
        collector(this.result)
        for (let i = 0; i < this.params.length; i++) {
            collector(this.params.$_get(i).typ)
        }
    }

    /**
     * Declares the lambda type in the context
     *
     * @param { TypeCreatorCtx } ctx current context
     * @throws { AssertionError } - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override declare(ctx: TypeCreatorCtx): void {
        if (this.ctxItemPtr !== 0) {
            throw new AssertionError("this.ctxItemPtr is not 0")
        }
        const ftPtr = TypeCreatorCtx.lambdaTypeCreate(ctx.ctxPtr, this.attrs.get())
        this.ctxItemPtr = ftPtr
        for (let i = 0; i < this.params.length; i++) {
            const par = this.params.$_get(i)
            ctx.checkError(TypeCreatorCtx.lambdaTypeAddParam(ftPtr, par.typ.getTypeDescriptor(), par.attrs.get()))
        }
        ctx.checkError(TypeCreatorCtx.lambdaTypeAddResult(ftPtr, this.result.getTypeDescriptor()))
        ctx.checkError(TypeCreatorCtx.lambdaTypeAdd(ftPtr))
    }

    /**
     * Defines the lambda type (no-op, already handled in declare)
     *
     * @param { TypeCreatorCtx } ctx current context
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override define(ctx: TypeCreatorCtx): void {
    }

    /**
     * Gets the target runtime linker
     *
     * @returns { RuntimeLinker } target linker
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    protected override getTargetLinker(): RuntimeLinker {
        return this.targetLinker
    }
}

/**
 * Interface that is used to freeze creators
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export interface Freezable {
    /**
     * Checks if this object is frozen
     *
     * @returns { boolean } true if frozen, false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    isFrozen(): boolean;

    /**
     * Freezes this object, preventing further modifications
     *
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    freeze(): void;
}

/**
 * Checks if modificatiosn are allowed
 * @param freezable to check if modifications are allowed
 * @throws TypeAPICreateError if it is frozen
 */
function checkNotFrozen(freezable: Freezable): void {
    if (freezable.isFrozen()) {
        throw new TypeAPICreateError("modifying frozen entity")
    }
}

/**
 * Internal creator that implements Freezable interface for TypeCreator
 * @extends TypeCreator
 * @implements Freezable
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
final class FreezableCreator extends TypeCreator implements Freezable {

    /**
     * Constructor
     *
     * @param { TypeCreator } creator creator to wrap
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(creator: TypeCreator) {
        super()
        this.typ = creator.typ
    }

    /**
     * Checks if this creator is frozen (type has been created)
     *
     * @returns { boolean } true if frozen, false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    override isFrozen(): boolean {
        return this.typ != undefined
    }

    /**
     * Gets the target runtime linker
     *
     * @returns { RuntimeLinker } target linker
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    protected override getTargetLinker(): RuntimeLinker {
        return getNearestNonBootRuntimeLinker()!
    }

    /**
     * Freezes the creator (no-op, freezing is determined by type creation)
     *
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override freeze(): void {}

    /**
     * Collects types (no-op, internal implementation)
     *
     * @param { CollectTypesCB } collector callback
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override collectTypes(collector: CollectTypesCB): void {}

    /**
     * Declares type (no-op, internal implementation)
     *
     * @param { TypeCreatorCtx } ctx context
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override declare(ctx: TypeCreatorCtx): void {}

    /**
     * Defines type (no-op, internal implementation)
     *
     * @param { TypeCreatorCtx } ctx context
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override define(ctx: TypeCreatorCtx): void {}
}

/**
 * Freezable used in features. It stores a boolean flag
 * @implements Freezable
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class FreezableImpl implements Freezable {
    private frozen: boolean = false

    /**
     * Checks if this implementation is frozen
     *
     * @returns { boolean } true if frozen, false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    override isFrozen(): boolean {
        return this.frozen
    }

    /**
     * Freezes this implementation
     *
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    override freeze(): void {
        this.frozen = true
    }
}

/**
 * Storage for attributes
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export class HasAttributes {
    /** Freezable that prevents modification */
    private frozen: Freezable
    /** attributes */
    private attrs: int = 0
    /** attributes that can be modified */
    private readonly allowed: int

    /**
     * constructor
     *
     * @param { Freezable } frozen Frozen configuration object.
     * @param { int } allowed Allowed operations bitmask.
     * @throws { TypeAPICreateError } - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(frozen: Freezable, allowed: int) {
        this.frozen = frozen
        this.allowed = allowed
    }

    /**
     * get
     *
     * @returns { int }
     * @throws { TypeAPICreateError } - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    get(): int {
        return this.attrs;
    }

    /**
     * has
     *
     * @param { int } attr Attribute flag to check.
     * @returns { boolean }
     * @throws { TypeAPICreateError } - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    has(attr: int): boolean {
        return (this.attrs & attr) != 0
    }

    /**
     * add
     *
     * @param { int } attr The attribute flag to add or modify.
     * @param { boolean } enabled True to enable the attribute, false to disable it.
     * @throws { TypeAPICreateError } - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    add(attr: int, enabled: boolean): void {
        if ((attr & this.allowed) != attr) {
            throw new TypeAPICreateError("disallowed attribute")
        }
        let newAttributes = this.attrs
        if (enabled) {
            newAttributes |= attr
        } else {
            newAttributes &= ~attr
        }
        if (this.attrs != newAttributes) {
            checkNotFrozen(this.frozen)
        }
        this.attrs = newAttributes
    }

    /**
     * add
     *
     * @param { int } attrs Attribute bitmask to apply.
     * @throws { TypeAPICreateError } - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    add(attrs: int): void {
        if ((attrs & this.allowed) != attrs) {
            throw new TypeAPICreateError("disallowed attributes")
        }
        if ((this.attrs & attrs) != attrs) {
            checkNotFrozen(this.frozen)
        }
        this.attrs = attrs
    }
}

/**
 * Storage for access modifier
 */
class HasAccessMod {
    frozen: Freezable
    accessMod: int = AccessModifier.PUBLIC

    constructor(frozen: Freezable) {
        this.frozen = frozen
    }

    get(): int {
        return this.accessMod;
    }

    set(accessMod: int): void {
        if (this.accessMod == accessMod) {
            return
        }
        checkNotFrozen(this.frozen)
        this.accessMod = accessMod
    }
}

/**
 * Creator for {@link Field}. Must be used as a part of {@link TypeCreator}.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class FieldCreator {
    /**
     * Field name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public name: string
    /**
     * Field type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public typ: TypeOrCreator

    /**
     * constructor
     *
     * @param { string } name new field name
     * @param { TypeOrCreator } typ new field type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    private constructor(name: string, typ: TypeOrCreator) {
        this.name = name
        this.attrs = new HasAttributes(this.frozen, AllowedAttributes.FIELD)
        this.accessMod = new HasAccessMod(this.frozen)
        this.typ = typ
    }

    private frozen: Freezable = new FreezableImpl()
    /**
     * Field attributes
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public attrs: HasAttributes
    private accessMod: HasAccessMod

    /**
     * constructor
     *
     * @param { string } name new field name
     * @param { Type } typ new field type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(name: string, typ: Type) {
        this(name, TypeOrCreator.from(typ))
    }

    /**
     * getAccessMod
     *
     * @returns { Int }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getAccessMod(): Int {
        return this.accessMod.get()
    }

    /**
     * constructor
     *
     * @param { string } name new field name
     * @param { TypeCreator } typ new field type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(name: string, typ: TypeCreator) {
        this(name, TypeOrCreator.from(typ))
    }

    /**
     * Sets all provided attributes to `true`
     *
     * @param { int } attrs attribute to set
     * @returns { FieldCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addAttributes(attrs: int): FieldCreator {
        this.attrs.add(attrs)
        return this;
    }

    /**
     * getFrozen
     *
     * @returns { Freezable }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getFrozen(): Freezable {
        return this.frozen
    }

    /**
     * Sets access modifier to provided
     *
     * @param { int } accessMod new access modifier
     * @returns { FieldCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addAccessModifier(accessMod: int): FieldCreator {
        this.accessMod.set(accessMod);
        return this;
    }
}

/**
 * Creator for {@link Parameter}. Must be used as a part of {@link MethodCreator}.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class ParameterCreator {
    /**
     * Parameter type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public typ: TypeOrCreator
    /**
     * Optional parameter name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public name?: string
    /**
     * Freezable instance
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public frozen: FreezableImpl = new FreezableImpl()
    /**
     * Parameter attributes
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public attrs: HasAttributes

    /**
     * constructor
     *
     * @param { Type } typ Type for this instance.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(typ: Type) {
        this(TypeOrCreator.from(typ))
    }

    /**
     * constructor
     *
     * @param { Type } typ Type for this instance.
     * @param { string } name name of parameter
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(typ: Type, name: string) {
        this(TypeOrCreator.from(typ))
        this.name = name
    }

    /**
     * constructor
     *
     * @param { TypeCreator } typ Type creator instance.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(typ: TypeCreator) {
        this(TypeOrCreator.from(typ))
    }

    /**
     * constructor
     *
     * @param { TypeOrCreator } typ Type instance or type creator.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(typ: TypeOrCreator) {
        this.typ = typ
        this.attrs = new HasAttributes(this.frozen, AllowedAttributes.PARAMETER)
    }

    /**
     * addName
     *
     * @param { string } name sets parameter name
     * @returns { ParameterCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addName(name: string): ParameterCreator {
        this.name = name
        return this;
    }

    /**
     * Sets all provided attributes to `true`
     *
     * @param { int } attrs attribute to set
     * @returns { ParameterCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addAttributes(attrs: int): ParameterCreator {
        this.attrs.add(attrs)
        return this;
    }

    /**
     * Adds attribute with same name
     *
     * @returns { ParameterCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addRest(): ParameterCreator {
        this.attrs.add(Attributes.REST, true)
        return this;
    }

    /**
     * Adds attribute with same name
     *
     * @returns { ParameterCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addOptional(): ParameterCreator {
        this.attrs.add(Attributes.OPTIONAL, true)
        return this;
    }
}

/**
 * Creator for {@link Method}. Must be used as a part of {@link ClassTypeCreator} or {@link InterfaceTypeCreatpr}.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class MethodCreator {
    /**
     * Method name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public name: string
    /**
     * Method parameters
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public params: Array<ParameterCreator> = new Array<ParameterCreator>();
    /**
     * Method return type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public result: TypeOrCreator = TypeOrCreator.from(VoidType.REF);
    private body?: CallableBody
    /**
     * Freezable instance
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public frozen: FreezableImpl = new FreezableImpl()
    /**
     * Method attributes
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public attrs: HasAttributes
    private accessMod: HasAccessMod

    /**
     * constructor
     *
     * @param { string } name method name, which can contain [a-zA-Z0-9] and be a valid identifier
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(name: string) {
        this.name = name
        this.attrs = new HasAttributes(this.frozen, AllowedAttributes.METHOD)
        this.accessMod = new HasAccessMod(this.frozen)
    }

    /**
     * Adds body to method
     *
     * @param { CallableBody | undefined } body new body; undefined can be used in abstract methods
     * @returns { MethodCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addBody(body: CallableBody | undefined): MethodCreator {
        checkNotFrozen(this.frozen)
        this.body = body
        return this;
    }

    /**
     * Appends parameter to `this` method type
     *
     * @param { ParameterCreator } param parameter to add
     * @returns { MethodCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addParameter(param: ParameterCreator): MethodCreator {
        checkNotFrozen(this.frozen)
        param.frozen.freeze()
        this.params.push(param)
        return this;
    }

    /**
     * Sets result
     *
     * @param { Type } typ type of result
     * @returns { MethodCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addResult(typ: Type): MethodCreator {
        return this.addResult(TypeOrCreator.from(typ))
    }

    /**
     * Sets result
     *
     * @param { TypeCreator } typ type of result
     * @returns { MethodCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addResult(typ: TypeCreator): MethodCreator {
        return this.addResult(TypeOrCreator.from(typ))
    }

    /**
     * Sets result
     *
     * @param { TypeOrCreator } typ type of result
     * @returns { MethodCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addResult(typ: TypeOrCreator): MethodCreator {
        checkNotFrozen(this.frozen)
        this.result = typ
        return this;
    }

    /**
     * Sets all provided attributes to `true`
     *
     * @param { int } attrs attribute to set
     * @returns { MethodCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addAttributes(attrs: int): MethodCreator {
        this.attrs.add(attrs)
        return this;
    }

    /**
     * Adds attribute with same name
     *
     * @returns { MethodCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addConstructor(): MethodCreator {
        return this.addConstructor(true)
    }

    /**
     * Adds attribute with same name
     *
     * @param { boolean } isCtor True for constructor, false for method.
     * @returns { MethodCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addConstructor(isCtor: boolean): MethodCreator {
        this.attrs.add(Attributes.CONSTRUCTOR, isCtor);
        return this;
    }

    /**
     * Adds attribute with same name
     *
     * @param { boolean } isStatic True for static, false for instance.
     * @returns { MethodCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addStatic(isStatic: boolean): MethodCreator {
        this.attrs.add(Attributes.STATIC, isStatic);
        return this;
    }

    /**
     * Adds attribute with same name
     *
     * @returns { MethodCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addGetter(): MethodCreator {
        this.attrs.add(Attributes.GETTER, true)
        return this;
    }

    /**
     * Adds attribute with same name
     *
     * @returns { MethodCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addSetter(): MethodCreator {
        this.attrs.add(Attributes.SETTER, true)
        return this;
    }

    /**
     * Adds attribute with same name
     *
     * @returns { MethodCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addAbstract(): MethodCreator {
        this.attrs.add(Attributes.ABSTRACT, true)
        return this;
    }

    /**
     * Sets access modifier to provided
     *
     * @param { int } accessMod new access modifier
     * @returns { MethodCreator }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addAccessModifier(accessMod: int): MethodCreator {
        this.accessMod.set(accessMod);
        return this;
    }

    /**
     * adds method to interface or class
     *
     * @param { TypeCreatorCtx } ctx Type creation context.
     * @param { long } classOrIfacePtr Pointer to target class/interface.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public addToCtxItemPtr(ctx: TypeCreatorCtx, classOrIfacePtr: long): void {
        const fptr = ctx.checkCtxItemPtr(TypeCreatorCtx.methodCreate(classOrIfacePtr, this.name, this.attrs.get()))
        ctx.checkError(TypeCreatorCtx.methodAddAccessMod(fptr, this.accessMod.get()))
        for (let a = 0; a < this.params.length; a++) {
            const par = this.params.$_get(a)
            let name = par.name ?? '' + a;
            ctx.checkError(TypeCreatorCtx.methodAddParam(fptr, par.typ.getTypeDescriptor(), name, par.attrs.get()))
        }
        ctx.checkError(TypeCreatorCtx.methodAddResult(fptr, this.result.getTypeDescriptor()))
        const isAbstract = this.attrs.has(Attributes.ABSTRACT)
        const hasBody = this.body != undefined
        if (isAbstract == hasBody) {
            ctx.checkError("only and only abstract methods don't have a body; abstract = " + isAbstract + "; body = " + (this.body == undefined ? "undefined" : this.body!.toString()))
        }
        if (!isAbstract) {
            const body = this.body!
            if (body instanceof CallableBodyMethod) {
                const callableBody = (body as CallableBodyMethod).getBody()
                ctx.checkError(TypeCreatorCtx.methodAddBodyFromMethod(fptr, callableBody.getType()))
            } else if (body instanceof CallableBodyFunction) {
                const bbody = (body as CallableBodyFunction).getBody()
                const id = ctx.registerObject(bbody as Object);
                const bbodyType = Type.of(bbody) as LambdaType
                ctx.checkError(TypeCreatorCtx.methodAddBodyFromLambda(fptr, id, bbodyType))
            } else if (body instanceof CallableBodyErasedFunction) {
                const bbody = (body as CallableBodyErasedFunction).getBody()
                const id = ctx.registerObject(bbody as Object);
                ctx.checkError(TypeCreatorCtx.methodAddBodyFromErasedLambda(fptr, id))
            } else if (body instanceof CallableBodyDefault) {
                ctx.checkError(TypeCreatorCtx.methodAddBodyDefault(fptr))
            } else {
                throw new TypeAPICreateError("body not supported" + this.body)
            }
        }
        ctx.checkError(TypeCreatorCtx.methodAdd(fptr))
    }
}

/**
 * Type that abstracts all supported function/lambda/method body-types
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export class CallableBody {
    /**
     * Makes class package-sealed
     *
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    protected constructor() {}
}

/**
 * Represents default body: does nothing for regular constructors (returns default value).
 * @extends CallableBody
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class CallableBodyDefault extends CallableBody {
    /**
     * Singleton instance of default callable body
     * @static
     * @readonly
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly INSTANCE = new CallableBodyDefault()

    /**
     * constructor
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor() {}
}

/**
 * Represents call to other method with forwarding all arguments as-is
 * @extends CallableBody
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class CallableBodyMethod extends CallableBody {
    private readonly body: Method

    /**
     * constructor
     *
     * @param { Method } m Method to wrap.
     * @throws { TypeAPICreateError } - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(m: Method) {
        super()
        this.body = m
    }

    /**
     * getBody
     *
     * @returns { Method }
     * @throws { TypeAPICreateError } - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getBody(): Method {
        return this.body
    }
}

/**
 * Represents call to lambda with forwarding all arguments as-is
 * @extends CallableBody
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class CallableBodyFunction extends CallableBody {
    private readonly body: Object

    /**
     * constructor
     *
     * @param { Object } m lambda to call
     * @throws { TypeAPICreateError } - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(m: Object) {
        super()
        if (!(Type.of(m) instanceof LambdaType)) {
            throw new TypeAPICreateError("can't encapsulate non-functional type")
        }
        this.body = m
    }

    /**
     * getBody
     *
     * @returns { Object }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getBody(): Object {
        return this.body
    }
}

/**
 * Represents call to lambda with boxing all arguments
 * @extends CallableBody
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class CallableBodyErasedFunction extends CallableBody {
    private /* readonly */ body: (recv: Any, args: FixedArray<Any>) => Any

    /**
     * constructor
     *
     * @param { function } m None
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(m: (recv: Any, args: FixedArray<Any>) => Any) {
        super()
        this.body = m
    }

    /**
     * getBody
     *
     * @returns { function }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getBody(): (recv: Any, args: FixedArray<Any>) => Any {
        return this.body
    }
}
