/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

package std.core;

export class TypeAPICreateException extends Exception {
    public constructor(msg: string) {
        super(msg)
    }
}

// TODO(kprokopenko): replace with Array<T> when it gets ready
final class MArray<T> {
    public constructor() {}

    public push(x: T): void {
        if (this.len == this.buf.length) {
            const newB = new T[this.buf.length * 2]
            for (let i = 0; i < this.buf.length; i++) {
                newB[i] = this.buf[i]
            }
            this.buf = newB
        }
        this.buf[this.len++] = x
    }

    public length(): int {
        return this.len
    }

    public at(i: int): T {
        if (i < 0 || i >= this.len) {
            throw new Error()
        }
        return this.buf[i]
    }

    public set(i: int, d: T): void {
        if (i < 0 || i >= this.len) {
            throw new Error()
        }
        this.buf[i] = d
    }

    public toArray(): T[] {
        const ret = new T[this.length()]
        for (let i = 0; i < this.length(); i++) {
            ret[i] = this.at(i)
        }
        return ret
    }

    len: int = 0
    buf: T[] = new T[2]
}

final class MSet<T> {
    public add(x: T): void {
        if (!this.has(x)) {
            this.elems.push(x)
        }
    }

    public elements(): T[] {
        return this.elems.toArray()
    }

    public has(x: T): boolean {
        for (let i = 0; i < this.elems.length(); i++) {
            if (this.elems.at(i) == x) {
                return true
            }
        }
        return false
    }

    private elems: MArray<T> = new MArray<T>()
}

type CollectTypesCB = (tc: TypeOrCreator) => void

export /* abstract */ class TypeCreator {
    internal typ: Type | null = null // not null if already created
    internal ctxItemPtr: long = 0 // resides in ctx, no need to free

    public create(): Type throws {
        if (this.typ == null) {
            const ctx = new TypeCreatorCtx()
            try {
                ctx.create(this)
            } finally {
                ctx.deinit()
            }
        }
        return this.typ!
    }

    internal /* abstract */ collectTypes(collector: CollectTypesCB): void throws {
        throw new Error("es2panda doesn't allow two abstract classes in overload")
    }
    internal /* abstract */ declare(ctx: TypeCreatorCtx): void throws {
        throw new Error("es2panda doesn't allow two abstract classes in overload")
    }
    internal /* abstract */ define(ctx: TypeCreatorCtx): void throws {
        throw new Error("es2panda doesn't allow two abstract classes in overload")
    }

    internal constructor() {}

    protected checkNotCreated(): void throws {
        if (this.typ != null) throw new TypeAPICreateException("type already created, no modifications allowed")
    }

    internal descriptor(): string {
        if (this.typ != null) {
            return this.typ!.td
        }
        assert this.ctxItemPtr != 0
        return TypeCreatorCtx.getTypeDescFromPointer(this.ctxItemPtr)
    }
}

abstract class TypeOrCreator {
    abstract descriptor(): string

    static from(typ: Type): TypeOrCreator {
        return new TypeOrCreator_Type(typ)
    }

    static from(typ: TypeCreator): TypeOrCreator {
        if (typ.typ != null) {
            return new TypeOrCreator_Type(typ.typ!)
        }
        return new TypeOrCreator_Creator(typ)
    }
}

final class TypeOrCreator_Type extends TypeOrCreator {
    private typ: Type

    override descriptor(): string {
        return this.typ.td
    }

    constructor(typ: Type) {
        this.typ = typ
    }
}

final class TypeOrCreator_Creator extends TypeOrCreator {
    typ: TypeCreator

    override descriptor(): string {
        return this.typ.descriptor()
    }

    constructor(typ: TypeCreator) {
        this.typ = typ
    }
}

class TypeCreatorCtx {
    internal ctxPtr: long
    private objects: MArray<Object> = new MArray<Object>();

    private static native createCtx(): long
    private static native destroyCtx(ctxPtr: long): void
    private static native commit(ctxPtr: long, objects: Object[]): string | null
    private static native getError(ctxPtr: long): string | null
    internal static native getObjectsArrayForCCtor(ctxPtr: long): Object[]

    internal static native getTypeDescFromPointer(ctxItemPtr: long): string

    internal static native classCreate(ctxPtr: long, name: string): long
    internal static native classSetBase(classPtr: long, base: TypeDesc): string | null
    internal static native classAddIface(classPtr: long, iface: TypeDesc): string | null
    internal static native classAddField(classPtr: long, name: string, field: TypeDesc, attrs: int, access: int): string | null

    internal static native interfaceCreate(ctxPtr: long, name: string): long
    internal static native interfaceAddBase(ifacePtr: long, base: TypeDesc): string | null

    internal static native lambdaTypeCreate(ctxPtr: long): long
    internal static native lambdaTypeAddParam(ftPtr: long, param: TypeDesc): string | null
    internal static native lambdaTypeAddResult(ftPtr: long, result: TypeDesc): string | null
    internal static native lambdaTypeAdd(ftPtr: long): string | null

    internal static native methodCreate(ctxItemPtr: long, name: string, attrs: int): long
    internal static native methodAddAccessMod(methPtr: long, access: int): string | null
    internal static native methodAddParam(methPtr: long, td: TypeDesc): string | null
    internal static native methodAddResult(methPtr: long, result: TypeDesc): string | null
    internal static native methodAddBodyFromMethod(methPtr: long, meth: string): string | null
    internal static native methodAddBodyFromLambda(methPtr: long, id: int, lambdaType: TypeDesc): string | null
    internal static native methodAddBodyFromErasedLambda(methPtr: long, id: int): string | null
    internal static native methodAddBodyDefault(methPtr: long): string | null
    internal static native methodAdd(methPtr: long): string | null

    internal checkError(): void throws {
        this.checkError(TypeCreatorCtx.getError(this.ctxPtr))
    }

    internal checkCtxItemPtr(ptr: long): long throws {
        if (ptr == 0) {
            this.checkError()
        }
        return ptr
    }

    internal deinit(): void {
        assert this.ctxPtr != 0
        TypeCreatorCtx.destroyCtx(this.ctxPtr)
        this.ctxPtr = 0
    }

    internal constructor() {
        this.ctxPtr = TypeCreatorCtx.createCtx()
    }

    public create(first: TypeCreator): void throws {
        const all = new MSet<TypeCreator>()
        const visited = new MSet<TypeCreator>()

        const order = new MArray<TypeCreator>()

        const push: CollectTypesCB = (tc: TypeOrCreator): void => {
            if (tc instanceof TypeOrCreator_Type) {
                return
            }
            const typeCreator = (tc as TypeOrCreator_Creator).typ
            if (typeCreator.typ != null) {
                return
            }
            if (visited.has(typeCreator)) {
                return
            }
            order.push(typeCreator)
            all.add(typeCreator)
        }

        push(new TypeOrCreator_Creator(first))

        let was = true
        while (was) {
            was = false
            const arr = all.elements()
            for (let i = 0; i != arr.length; i++) {
                const el: TypeCreator = arr[i] as TypeCreator
                if (visited.has(el)) {
                    continue
                }
                was = true
                visited.add(el)
                el.collectTypes(push)
            }
        }

        for (let i = 0; i < order.length() / 2; i++) {
            const j = order.length() - 1 - i
            const tmp = order.at(i)
            order.set(i, order.at(j))
            order.set(j, tmp)
        }

        for (let i = 0; i < order.length(); i++) {
            order.at(i).declare(this)
        }

        for (let i = 0; i < order.length(); i++) {
            order.at(i).define(this)
        }

        this.checkError()

        this.checkError(TypeCreatorCtx.commit(this.ctxPtr, this.objects.toArray() as Object[]))

        for (let i = 0; i < order.length(); i++) {
            const tc = order.at(i)
            tc.typ = Type.resolve(TypeCreatorCtx.getTypeDescFromPointer(tc.ctxItemPtr))
        }
    }

    internal registerObject(o: Object): int throws {
        const r = this.objects.length()
        this.objects.push(o)
        return r
    }

    internal checkError(arg: string | null): void throws {
        if (arg != null) {
            throw new TypeAPICreateException(arg as string)
        }
    }
}

function addMethodToCtxItemPtr(ctx: TypeCreatorCtx, classPtr: long, m: MethodCreator): void throws {
    const fptr = ctx.checkCtxItemPtr(TypeCreatorCtx.methodCreate(classPtr, m.name, m.attrs.get()))
    ctx.checkError(TypeCreatorCtx.methodAddAccessMod(fptr, m.accessMod.get()))
    for (let a = 0; a < m.params.length(); a++) {
        ctx.checkError(TypeCreatorCtx.methodAddParam(fptr, m.params.at(a).descriptor()))
    }
    ctx.checkError(TypeCreatorCtx.methodAddResult(fptr, m.result.descriptor()))
    const isAbstract = m.attrs.has(Attributes.ABSTRACT)
    const hasBody = m.body != null
    if (isAbstract == hasBody) {
        ctx.checkError("only and only abstract methods don't have a body; abstract = " + isAbstract + "; body = " + (m.body == null ? "null" : m.body.toString()))
    }
    if (!isAbstract) {
        const body = m.body!
        if (body instanceof CallableBodyMethod) {
            ctx.checkError(TypeCreatorCtx.methodAddBodyFromMethod(fptr, (body as CallableBodyMethod).body.getTD()))
        } else if (body instanceof CallableBodyFunction) {
            const bbody = (body as CallableBodyFunction).body
            const id = ctx.registerObject(bbody as Object);
            ctx.checkError(TypeCreatorCtx.methodAddBodyFromLambda(fptr, id, Type.of(bbody).td))
        } else if (body instanceof CallableBodyErasedFunction) {
            const bbody = (body as CallableBodyErasedFunction).body
            const id = ctx.registerObject(bbody as Object);
            ctx.checkError(TypeCreatorCtx.methodAddBodyFromErasedLambda(fptr, id))
        } else if (body instanceof CallableBodyDefault) {
            ctx.checkError(TypeCreatorCtx.methodAddBodyDefault(fptr))
        } else {
            throw new TypeAPICreateException("body not supported" + m.body)
        }
    }
    ctx.checkError(TypeCreatorCtx.methodAdd(fptr))
}

export final class ClassTypeCreator extends TypeCreator {
    private base: TypeOrCreator = TypeOrCreator.from(ObjectType)
    private name: string
    private ifaces: MArray<TypeOrCreator> = new MArray<TypeOrCreator>()
    private fields: MArray<FieldCreator> = new MArray<FieldCreator>()
    private methods: MArray<MethodCreator> = new MArray<MethodCreator>()

    public constructor(name: string) {
        super()
        this.name = name
    }

    public override create(): ClassType throws {
        return super.create() as ClassType
    }

    public addBase(base: ClassType): ClassTypeCreator throws {
        this.checkNotCreated()
        this.base = TypeOrCreator.from(base)
        return this;
    }

    public addBase(base: ClassTypeCreator): ClassTypeCreator throws {
        this.checkNotCreated()
        this.base = TypeOrCreator.from(base)
        return this;
    }

    public addInterface(iface: InterfaceType): ClassTypeCreator throws {
        this.ifaces.push(TypeOrCreator.from(iface))
        return this;
    }

    public addInterface(iface: InterfaceTypeCreator): ClassTypeCreator throws {
        this.ifaces.push(TypeOrCreator.from(iface))
        return this;
    }

    public addField(fld: FieldCreator): ClassTypeCreator throws {
        fld.frozen.freeze()
        this.fields.push(fld)
        return this;
    }

    internal /* override */ collectTypes(collector: CollectTypesCB): void throws {
        collector(this.base)
        for (let i = 0; i < this.ifaces.length(); i++) {
            collector(this.ifaces.at(i))
        }
        for (let i = 0; i < this.fields.length(); i++) {
            collector(this.fields.at(i).typ)
        }
        for (let i = 0; i < this.methods.length(); i++) {
            const m = this.methods.at(i)
            collector(m.result)
            for (let i = 0; i < m.params.length(); i++) {
                collector(m.params.at(i))
            }
        }
    }

    internal /* override */ declare(ctx: TypeCreatorCtx): void throws {
        assert this.ctxItemPtr == 0
        this.ctxItemPtr = ctx.checkCtxItemPtr(TypeCreatorCtx.classCreate(ctx.ctxPtr, this.name))
    }

    internal /* override */ define(ctx: TypeCreatorCtx): void throws {
        const classPtr = this.ctxItemPtr
        ctx.checkError(TypeCreatorCtx.classSetBase(classPtr, this.base.descriptor()))
        for (let i = 0; i < this.ifaces.length(); i++) {
            ctx.checkError(TypeCreatorCtx.classAddIface(classPtr, this.ifaces.at(i).descriptor()))
        }
        for (let i = 0; i < this.fields.length(); i++) {
            const f = this.fields.at(i);
            ctx.checkError(TypeCreatorCtx.classAddField(classPtr, f.name, f.typ.descriptor(), f.attrs.get(), f.accessMod.get()))
        }
        for (let i = 0; i < this.methods.length(); i++) {
            const m = this.methods.at(i)
            addMethodToCtxItemPtr(ctx, classPtr, m)
        }
    }

    public addMethod(mb: MethodCreator): ClassTypeCreator throws {
        mb.frozen.freeze()
        this.methods.push(mb)
        return this;
    }
}

export final class InterfaceTypeCreator extends TypeCreator {
    private name: string
    private bases: MArray<TypeOrCreator> = new MArray<TypeOrCreator>()
    private methods: MArray<MethodCreator> = new MArray<MethodCreator>()

    public constructor(name: string) {
        super()
        this.name = name
    }

    public override create(): InterfaceType throws {
        return super.create() as InterfaceType
    }

    public addBase(base: InterfaceType): InterfaceTypeCreator throws {
        this.checkNotCreated()
        this.bases.push(TypeOrCreator.from(base))
        return this;
    }

    public addBase(base: InterfaceTypeCreator): InterfaceTypeCreator throws {
        this.checkNotCreated()
        this.bases.push(TypeOrCreator.from(base))
        return this;
    }

    public addField(fld: FieldCreator): InterfaceTypeCreator throws {
        const getter = new MethodCreator(fld.name).addResult(fld.typ)
        this.addMethod(getter)
        if (!fld.attrs.has(Attributes.READONLY)) {
            const setter = new MethodCreator(fld.name).addParameter(fld.typ)
            this.addMethod(setter)
        }
        return this;
    }

    public addMethod(mc: MethodCreator): InterfaceTypeCreator throws {
        if ((mc.attrs.has(Attributes.STATIC))) {
            throw new TypeAPICreateException("interface can't have static method " + mc)
        }
        mc.addAttributes(Attributes.ABSTRACT)
        mc.addAccessModifier(AccessModifier.PUBLIC)
        mc.frozen.freeze()
        this.methods.push(mc)
        return this;
    }

    internal /* override */ collectTypes(collector: CollectTypesCB): void throws {
        for (let i = 0; i < this.bases.length(); i++) {
            collector(this.bases.at(i))
        }
        for (let i = 0; i < this.methods.length(); i++) {
            const m = this.methods.at(i)
            collector(m.result)
            for (let i = 0; i < m.params.length(); i++) {
                collector(m.params.at(i))
            }
        }
    }

    internal /* override */ define(ctx: TypeCreatorCtx): void throws {
        const classPtr = this.ctxItemPtr
        for (let i = 0; i < this.bases.length(); i++) {
            ctx.checkError(TypeCreatorCtx.interfaceAddBase(classPtr, this.bases.at(i).descriptor()))
        }
        for (let i = 0; i < this.methods.length(); i++) {
            const m = this.methods.at(i)
            addMethodToCtxItemPtr(ctx, classPtr, m)
        }
    }

    internal /* override */ declare(ctx: TypeCreatorCtx): void throws {
        assert this.ctxItemPtr == 0
        this.ctxItemPtr = ctx.checkCtxItemPtr(TypeCreatorCtx.interfaceCreate(ctx.ctxPtr, this.name))
    }
}

export final class LambdaTypeCreator extends TypeCreator {
    internal params: MArray<TypeOrCreator> = new MArray<TypeOrCreator>();
    internal result: TypeOrCreator = TypeOrCreator.from(VoidType.REF)

    public constructor() {}

    public override create(): LambdaType throws {
        return super.create() as LambdaType
    }

    public addParameter(typ: Type): LambdaTypeCreator throws {
        return this.addParameter(TypeOrCreator.from(typ));
    }

    public addParameter(typ: TypeCreator): LambdaTypeCreator throws {
        return this.addParameter(TypeOrCreator.from(typ));
    }

    internal addParameter(typ: TypeOrCreator): LambdaTypeCreator throws {
        this.checkNotCreated()
        this.params.push(typ)
        return this;
    }

    public addResult(typ: Type): LambdaTypeCreator throws {
        return this.addResult(TypeOrCreator.from(typ));
    }

    public addResult(typ: TypeCreator): LambdaTypeCreator throws {
        return this.addResult(TypeOrCreator.from(typ));
    }

    internal addResult(typ: TypeOrCreator): LambdaTypeCreator throws {
        this.checkNotCreated()
        this.result = typ
        return this;
    }

    internal /* override */ collectTypes(collector: CollectTypesCB): void throws {
        collector(this.result)
        for (let i = 0; i < this.params.length(); i++) {
            collector(this.params.at(i))
        }
    }

    internal /* override */ declare(ctx: TypeCreatorCtx): void throws {
        assert this.ctxItemPtr == 0
        const ftPtr = TypeCreatorCtx.lambdaTypeCreate(ctx.ctxPtr)
        this.ctxItemPtr = ftPtr
        for (let i = 0; i < this.params.length(); i++) {
            const td = this.params.at(i).descriptor()
            ctx.checkError(TypeCreatorCtx.lambdaTypeAddParam(ftPtr, td))
        }
        ctx.checkError(TypeCreatorCtx.lambdaTypeAddResult(ftPtr, this.result.descriptor()))
        ctx.checkError(TypeCreatorCtx.lambdaTypeAdd(ftPtr))
    }

    internal /* override */ define(ctx: TypeCreatorCtx): void throws {
    }
}

class Freezable {
    frozen: boolean = false

    freeze(): void {
        this.frozen = true
    }

    checkNotFrozen(): void throws {
        if (this.frozen) { throw new TypeAPICreateException("modifying frozen entity") }
    }
}

class HasAttributes {
    frozen: Freezable
    attrs: int = 0

    constructor(frozen: Freezable) {
        this.frozen = frozen
    }

    get(): int {
        return this.attrs;
    }

    has(attr: int): boolean {
        return (this.attrs & attr) != 0
    }

    add(attr: int, enabled: boolean): void throws {
        let newAttributes = this.attrs
        if (enabled) {
            newAttributes |= attr
        } else {
            newAttributes &= ~attr
        }
        if (this.attrs != newAttributes) {
            this.frozen.checkNotFrozen()
        }
        this.attrs = newAttributes
    }

    add(attrs: int): void throws {
        if ((this.attrs & attrs) != attrs) {
            this.frozen.checkNotFrozen()
        }
        this.attrs |= attrs
    }
}

class HasAccessMod {
    frozen: Freezable
    accessMod: int = AccessModifier.PUBLIC

    constructor(frozen: Freezable) {
        this.frozen = frozen
    }

    get(): int {
        return this.accessMod;
    }

    set(accessMod: int): void throws {
        if (this.accessMod == accessMod) {
            return
        }
        this.frozen.checkNotFrozen()
        this.accessMod = accessMod
    }
}

export final class FieldCreator {
    internal name: string
    internal typ: TypeOrCreator
    internal frozen: Freezable = new Freezable()
    internal attrs: HasAttributes = new HasAttributes(this.frozen)
    internal accessMod: HasAccessMod = new HasAccessMod(this.frozen)

    public constructor(name: string, typ: Type) {
        this.name = name
        this.typ = TypeOrCreator.from(typ)
    }

    public constructor(name: string, typ: TypeCreator) {
        this.name = name
        this.typ = TypeOrCreator.from(typ)
    }

    public addAttribute(attr: int, enabled: boolean): FieldCreator throws {
        this.attrs.add(attr, enabled)
        return this;
    }
    public addAttributes(attrs: int): FieldCreator throws {
        this.attrs.add(attrs)
        return this;
    }

    public addAccessModifier(accessMod: int): FieldCreator throws {
        this.accessMod.set(accessMod);
        return this;
    }
}

export final class MethodCreator {
    internal name: string
    internal params: MArray<TypeOrCreator> = new MArray<TypeOrCreator>();
    internal result: TypeOrCreator = TypeOrCreator.from(VoidType.REF)
    internal body: CallableBody | null = null
    internal frozen: Freezable = new Freezable()
    internal attrs: HasAttributes = new HasAttributes(this.frozen)
    internal accessMod: HasAccessMod = new HasAccessMod(this.frozen)

    public constructor(name: string) {
        this.name = name
    }

    public addBody(body: CallableBody | null): MethodCreator throws {
        this.frozen.checkNotFrozen()
        this.body = body
        return this;
    }

    public addParameter(typ: Type): MethodCreator throws {
        return this.addParameter(TypeOrCreator.from(typ));
    }

    public addParameter(typ: TypeCreator): MethodCreator throws {
        return this.addParameter(TypeOrCreator.from(typ));
    }

    public addParameter(typ: TypeOrCreator): MethodCreator throws {
        this.frozen.checkNotFrozen()
        this.params.push(typ)
        return this;
    }

    public addResult(typ: Type): MethodCreator throws {
        return this.addResult(TypeOrCreator.from(typ))
    }

    public addResult(typ: TypeCreator): MethodCreator throws {
        return this.addResult(TypeOrCreator.from(typ))
    }

    internal addResult(typ: TypeOrCreator): MethodCreator throws {
        this.frozen.checkNotFrozen()
        this.result = typ
        return this;
    }


    public addAttribute(attr: int, enabled: boolean): MethodCreator throws {
        this.attrs.add(attr, enabled)
        return this;
    }

    public addAttributes(attrs: int): MethodCreator throws {
        this.attrs.add(attrs)
        return this;
    }

    public addConstructor(): MethodCreator throws {
        return this.addConstructor(true)
    }

    public addConstructor(isCtor: boolean): MethodCreator throws {
        return this.addAttribute(Attributes.CONSTRUCTOR, isCtor);
    }

    public addStatic(isStatic: boolean): MethodCreator throws {
        return this.addAttribute(Attributes.STATIC, isStatic);
    }


    public addAccessModifier(accessMod: int): MethodCreator throws {
        this.accessMod.set(accessMod);
        return this;
    }
}

export class CallableBody {
    internal constructor() {}
}

export final class CallableBodyDefault extends CallableBody {
    public static readonly INSTANCE = new CallableBodyDefault()

    public constructor() {}
}

export final class CallableBodyMethod extends CallableBody {
    internal body: Method

    public constructor(m: Method) {
        super()
        this.body = m
    }
}

export final class CallableBodyFunction extends CallableBody {
    internal body: Object

    public constructor(m: Object) throws {
        super()
        if (!(Type.of(m) instanceof LambdaType)) {
            throw new TypeAPICreateException("can't encapsulate non-functional type")
        }
        this.body = m
    }
}

export final class CallableBodyErasedFunction extends CallableBody {
    internal body: (args: Object[]) => Object

    public constructor(m: (args: Object[]) => Object) {
        super()
        this.body = m
    }
}
