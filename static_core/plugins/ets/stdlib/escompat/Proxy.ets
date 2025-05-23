/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

export final class Proxy {
    private constructor() {}

    private static readonly INSTANCE = new Proxy()

    // Note(ivan-tyulyandin): invalid ETS names to prevent collision with user code
    private static readonly PROXY_CLASS_NAME_SUFFIX = "@Proxy"
    private static readonly PROXY_INSTANCE_TARGET_FIELD = "@target"
    private static readonly PROXY_INSTANCE_HANDLER_FIELD = "@handler"

    private static readonly CTOR_METHOD_NAME = "constructor"

    private static readonly GETTER_METHOD_PREFIX = "<get>"
    private static readonly GETTER_METHOD_PREFIX_LENGTH = Proxy.GETTER_METHOD_PREFIX.getLength()

    private static readonly SETTER_METHOD_PREFIX = "<set>"
    private static readonly SETTER_METHOD_PREFIX_LENGTH = Proxy.SETTER_METHOD_PREFIX.getLength()

    static tryGetTarget(obj: Object): NullishType {
        return Reflect.get(obj, Proxy.PROXY_INSTANCE_TARGET_FIELD)
    }

    static tryGetHandler(obj: Object): NullishType {
        return Reflect.get(obj, Proxy.PROXY_INSTANCE_HANDLER_FIELD)
    }

    static create<T extends Object>(target: T, handler: ProxyHandler<T>): T {
        const targetType = Type.of(target) as ClassType
        // ignore proxy generation for system classes 'std.core' and 'escompat' packages
        const targetTypeName = targetType.getName()
        if (targetTypeName.startsWith("std.core.") || targetTypeName.startsWith("escompat.")) {
            return target
        }

        const handlerType = Type.of(handler) as ClassType
        const targetLinker = Class.of(target).getLinker()
        const proxyType = Proxy.INSTANCE.createProxyType(targetType, handlerType, targetLinker)

        const proxyInstance = proxyType.make() as T
        Reflect.set(proxyInstance, Proxy.PROXY_INSTANCE_TARGET_FIELD, target)
        Reflect.set(proxyInstance, Proxy.PROXY_INSTANCE_HANDLER_FIELD, handler)

        return proxyInstance
    }

    static create<T>(target: Array<T>, handler: ArrayProxyHandler<T>): Array<T> {
        return Proxy.INSTANCE.makeArrayProxyInstance(target, handler)
    }

    static create<T>(target: ReadonlyArray<T>, handler: ReadonlyArrayProxyHandler<T>): ReadonlyArray<T> {
        return new ReadonlyArrayProxy<T>(target, handler)
    }

    private getProxyClassName(targetType: ClassType): string {
        return targetType.getName() + Proxy.PROXY_CLASS_NAME_SUFFIX
    }

    private createProxyType(targetType: ClassType, handlerType: ClassType, targetLinker: RuntimeLinker): ClassType {
        const proxyClassName = this.getProxyClassName(targetType)
        const proxyType = targetLinker.getType(proxyClassName)
        if (proxyType) {
            return proxyType as ClassType
        }

        const targetDefaultCtor = this.findDefaultConstructor(targetType)
        if (targetDefaultCtor === undefined) {
            throw new TypeError("unable to create proxy for the class without default constructor")
        }

        const proxyTypeCreator = new ClassTypeCreator(proxyClassName)
                .addBaseType(targetType)

        const targetFieldCreator = new FieldCreator(Proxy.PROXY_INSTANCE_TARGET_FIELD, targetType)
                .addAccessModifier(AccessModifier.PRIVATE)
        proxyTypeCreator.addField(targetFieldCreator)

        const handlerFieldCreator = new FieldCreator(Proxy.PROXY_INSTANCE_HANDLER_FIELD, handlerType)
                .addAccessModifier(AccessModifier.PRIVATE)
        proxyTypeCreator.addField(handlerFieldCreator)

        const ctorCreator = new MethodCreator(Proxy.CTOR_METHOD_NAME)
                .addConstructor()
                .addBody(new CallableBodyMethod(targetDefaultCtor))

        proxyTypeCreator.addMethod(ctorCreator)

        this.generateProxyMethods(targetType, proxyTypeCreator)

        return proxyTypeCreator.create() as ClassType
    }

    private createArrayProxyType(arrayProxyBaseType: ClassType, handlerType: InterfaceType, targetLinker: RuntimeLinker): ClassType {
        const proxyClassName = this.getProxyClassName(arrayProxyBaseType)
        const proxyType = targetLinker.getType(proxyClassName)
        if (proxyType) {
            return proxyType as ClassType
        }

        const proxyTypeCreator = new ClassTypeCreator(proxyClassName)
                .addBaseType(arrayProxyBaseType)

        const arrayProxyCtor = this.findArrayProxyConstructor(arrayProxyBaseType)
        if (arrayProxyCtor !== undefined) {
            const ctorCreator = new MethodCreator(Proxy.CTOR_METHOD_NAME)
                    .addConstructor()
                    .addParameter(new ParameterCreator(ArrayProxy.ARRAY_TYPE))
                    .addParameter(new ParameterCreator(handlerType))
                    .addBody(new CallableBodyMethod(arrayProxyCtor))

            proxyTypeCreator.addMethod(ctorCreator)

            this.generateArrayProxyMethods(proxyTypeCreator)

            return proxyTypeCreator.create() as ClassType
        } else {
            throw new AssertionError("failed to find array proxy base type constructor")
        }
    }

    private makeArrayProxyInstance<T>(target: Array<T>, handler: ArrayProxyHandler<T>): Array<T> {
        const proxies: ArrayProxy<T>[] = []
        const arrayProxyBaseType = (Type.of(proxies) as ArrayType).getElementType() as ClassType

        const handlers: ArrayProxyHandler<T>[] = []
        const handlerType = (Type.of(handlers) as ArrayType).getElementType() as InterfaceType

        const targetLinker = getNearestNonBootRuntimeLinker()!
        const arrayProxyType = this.createArrayProxyType(arrayProxyBaseType, handlerType, targetLinker)
        return arrayProxyType.make([target, handler]) as Array<T>
    }

    private findDefaultConstructor(type: ClassType): Method | undefined {
        const ctorsCount = type.getConstructorsNum()
        for (let ctorIdx = 0; ctorIdx < ctorsCount; ctorIdx++) {
            const ctor = type.getConstructor(ctorIdx)
            const ctorType = ctor.getType()
            if (ctorType.getParametersNum() == 0) {
                return ctor
            }
        }

        return undefined
    }

    private findArrayProxyConstructor(arrayProxyType: ClassType): Method | undefined {
        const ctorsCount = arrayProxyType.getConstructorsNum()
        for (let ctorIdx = 0; ctorIdx < ctorsCount; ctorIdx++) {
            const ctor = arrayProxyType.getConstructor(ctorIdx)
            const ctorType = ctor.getType()
            if (ctorType.getParametersNum() == 2) {
                return ctor
            }
        }

        return undefined
    }

    private generateArrayProxyMethods(proxyTypeCreator: ClassTypeCreator): void {
        for (let methodIdx = 0; methodIdx < ArrayProxy.ARRAY_TYPE_METHODS_COUNT; methodIdx++) {
            const method = ArrayProxy.ARRAY_TYPE_METHODS[methodIdx]

            const isGetter = method.isGetter()
            const isSetter = method.isSetter()
            const isInherited = method.isInherited()
            const isStatic = method.isStatic()
            const isFinal = method.isFinal()

            const methodName = method.getName()
            const isIgnored = ArrayProxy.ARRAY_PROXY_DEFINED_METHODS.has(methodName) || methodName.startsWith("lambda$")
            const isPublic = method.getAccessModifier() == AccessModifier.PUBLIC

            if (isGetter || isSetter || isInherited || isStatic || isFinal || !isPublic || isIgnored) {
                continue
            }

            this.generateArrayMethodInvocationProxy(proxyTypeCreator, method)
        }
    }

    private generateProxyMethods(targetType: ClassType, proxyTypeCreator: ClassTypeCreator): void {
        while (!targetType.equals(ObjectType)) {
            const methodsCount = targetType.getMethodsNum()
            for (let methodIdx = 0; methodIdx < methodsCount; methodIdx++) {
                const method = targetType.getMethod(methodIdx)

                const isGetter = method.isGetter()
                if (isGetter) {
                    this.generateGetterProxy(proxyTypeCreator, method)
                }

                const isSetter = method.isSetter()
                if (isSetter) {
                    this.generateSetterProxy(proxyTypeCreator, method)
                }

                const isInherited = method.isInherited()
                const isStatic = method.isStatic()
                const isFinal = method.isFinal()
                const isAbstract = method.isAbstract()

                const accessModifier = method.getAccessModifier()
                const isPublic = method.getAccessModifier() == AccessModifier.PUBLIC

                if (!(isGetter || isSetter || isInherited || isStatic || isFinal || isAbstract || !isPublic)) {
                    this.generateMethodInvocationProxy(proxyTypeCreator, method)
                }
            }

            targetType = targetType.getBaseType()
        }
    }

    private handlerGet<T extends Object>(target: T, handler: ProxyHandler<T>, propName: string): NullishType {
        return handler.get(target, propName)
    }

    private generateGetterProxy(
            proxyTypeCreator: ClassTypeCreator,
            getter: Method): void {

        const propertyName = getter.getName().substring(Proxy.GETTER_METHOD_PREFIX_LENGTH)
        const getterType = getter.getType()

        const getterCreator = new MethodCreator(getter.getName())
                .addAccessModifier(getter.getAccessModifier())
                .addResult(getterType.getResultType())
                .addBody(new CallableBodyFunction((proxy: Object): NullishType => {
                    const handler = Reflect.get(proxy, Proxy.PROXY_INSTANCE_HANDLER_FIELD)
                    if (handler instanceof ProxyHandler) {
                        const target = Reflect.get(proxy, Proxy.PROXY_INSTANCE_TARGET_FIELD)
                        // using "wrapper" method to "capture" <T> after smartcast
                        return this.handlerGet(target, handler, propertyName)
                    }

                    throw new TypeError("unexpected proxy handler type: " + Type.of(handler).getName())
                }))

        proxyTypeCreator.addMethod(getterCreator)
    }

    private handlerSet<T extends Object>(target: T, handler: ProxyHandler<T>, propName: string, propVal: NullishType): boolean {
        return handler.set(target, propName, propVal)
    }

    private generateSetterProxy(
            proxyTypeCreator: ClassTypeCreator,
            setter: Method): void {

        const propertyName = setter.getName().substring(Proxy.SETTER_METHOD_PREFIX_LENGTH)
        const setterType = setter.getType()

        const setterCreator = new MethodCreator(setter.getName())
            .addAccessModifier(setter.getAccessModifier())
            .addResult(setterType.getResultType())
            .addParameter(new ParameterCreator(setterType.getParameter(0).getType()))
            .addBody(new CallableBodyFunction((proxy: Object, value: NullishType): void => {
                const handler = Reflect.get(proxy, Proxy.PROXY_INSTANCE_HANDLER_FIELD)
                if (handler instanceof ProxyHandler) {
                    const target = Reflect.get(proxy, Proxy.PROXY_INSTANCE_TARGET_FIELD)
                    // using "wrapper" method to "capture" <T> after smartcast
                    const success = this.handlerSet(target, handler, propertyName, value)
                    if (!success) {
                        throw new TypeError(`proxy set handler returned false for propety '"${propertyName}"'`)
                    }
                } else {
                    throw new TypeError("unexpected proxy handler type: " + Type.of(handler).getName())
                }
            }))

        proxyTypeCreator.addMethod(setterCreator)
    }

    private handlerInvoke<T extends Object>(target: T, handler: ProxyHandler<T>, method: Method, args: NullishType[]): NullishType {
        return handler.invoke(target, method, args)
    }

    private generateMethodInvocationProxy(proxyTypeCreator: ClassTypeCreator, method: Method): void {
        const methodType = method.getType()
        const methodProxyCreator = new MethodCreator(method.getName())

        for (let paramIdx = 0; paramIdx < methodType.getParametersNum(); paramIdx++) {
            methodProxyCreator.addParameter(new ParameterCreator(methodType.getParameter(paramIdx).getType()))
        }

        methodProxyCreator
                .addResult(methodType.getResultType())
                .addBody(new CallableBodyErasedFunction((proxy: NullishType, args: NullishType[]): NullishType => {
                    const proxyInstance = proxy as Object
                    const handler = Reflect.get(proxyInstance, Proxy.PROXY_INSTANCE_HANDLER_FIELD)
                    if (handler instanceof ProxyHandler) {
                        const target = Reflect.get(proxyInstance, Proxy.PROXY_INSTANCE_TARGET_FIELD)
                        return this.handlerInvoke(target, handler, method, args)
                    }

                    throw new TypeError("unexpected proxy handler type: " + Type.of(handler).getName())
                }))

        proxyTypeCreator.addMethod(methodProxyCreator)
    }

    private wrapLambda(lambdaObj: Object): Object {
        const lambdaType = Type.of(lambdaObj) as LambdaType
        if (lambdaObj instanceof Function0) {
            return this.wrapFn0(lambdaObj)
        } else if (lambdaObj instanceof Function1) {
            return this.wrapFn1(lambdaObj)
        } else if (lambdaObj instanceof Function2) {
            return this.wrapFn2(lambdaObj)
        } else if (lambdaObj instanceof Function3) {
            return this.wrapFn3(lambdaObj)
        } else if (lambdaObj instanceof Function4) {
            return this.wrapFn4(lambdaObj)
        } else {
            throw new TypeError("unsuppored function arity: " + lambdaType.getParametersNum())
        }
    }

    private wrapFn0<R>(fn: Function0<R>): Function0<R> {
        return (): R => fn.invoke0()
    }

    private wrapFn1<P1, R>(fn: Function1<P1, R>): Function1<P1, R> {
        return (p1: P1): R => fn.invoke1(p1)
    }

    private wrapFn2<P1, P2, R>(fn: Function2<P1, P2, R>): Function2<P1, P2, R> {
        return (p1: P1, p2: P2): R => fn.invoke2(p1, p2)
    }

    private wrapFn3<P1, P2, P3, R>(fn: Function3<P1, P2, P3, R>): Function3<P1, P2, P3, R> {
        return (p1: P1, p2: P2, p3: P3): R => fn.invoke3(p1, p2, p3)
    }

    private wrapFn4<P1, P2, P3, P4, R>(fn: Function4<P1, P2, P3, P4, R>): Function4<P1, P2, P3, P4, R> {
        return (p1: P1, p2: P2, p3: P3, p4: P4): R => fn.invoke4(p1, p2, p3, p4)
    }

    private arrayHandlerInvoke<T>(arrayProxy: ArrayProxy<T>, method: Method, args: NullishType[]): NullishType {
        const methodArgs = new NullishType[args.length]

        for (let argIdx = 0; argIdx < args.length; argIdx++) {
            const arg = args[argIdx]

            const argType = Type.of(arg)
            if (argType instanceof LambdaType) {
                methodArgs[argIdx] = this.wrapLambda(arg as Object)
            } else {
                methodArgs[argIdx] = arg
            }
        }

        return method.invoke(arrayProxy.target, methodArgs)
    }

    private generateArrayMethodInvocationProxy(proxyTypeCreator: ClassTypeCreator, method: Method): void {
        const methodType = method.getType()
        const methodProxyCreator = new MethodCreator(method.getName())

        for (let paramIdx = 0; paramIdx < methodType.getParametersNum(); paramIdx++) {
            methodProxyCreator.addParameter(new ParameterCreator(methodType.getParameter(paramIdx).getType()))
        }

        methodProxyCreator
                .addResult(methodType.getResultType())
                .addBody(new CallableBodyErasedFunction((arrayProxy: NullishType, args: NullishType[]): NullishType => {
                    if (arrayProxy instanceof ArrayProxy) {
                        return this.arrayHandlerInvoke(arrayProxy, method, args)
                    }

                    throw new AssertionError("unexpected array proxy type: " + Type.of(arrayProxy).getName())
                }))

        proxyTypeCreator.addMethod(methodProxyCreator)
    }
}

export interface ProxyHandler<T extends Object> {
    get(target: T, name: string): NullishType

    set(target: T, name: string, value: NullishType): boolean

    invoke(target: T, method: Method, args: NullishType[]): NullishType
}

export abstract class DefaultProxyHandler<T extends Object> implements ProxyHandler<T> {
    override get(target: T, name: string): NullishType {
        return Reflect.get(target, name)
    }

    override set(target: T, name: string, value: NullishType): boolean {
        return Reflect.set(target, name, value)
    }

    override invoke(target: T, method: Method, args: NullishType[]): NullishType {
        return method.invoke(target, args)
    }
}

export interface ArrayProxyHandler<T> extends ProxyHandler<Array<T>> {
    get(target: Array<T>, index: int): T

    set(target: Array<T>, index: int, value: T): boolean
}

export abstract class DefaultArrayProxyHandler<T> extends DefaultProxyHandler<Array<T>> implements ArrayProxyHandler<T> {
    override get(target: Array<T>, name: string): NullishType {
        if (name == ArrayProxy.LENGTH_PROP_NAME) {
            return target.length
        }

        throw new AssertionError("unexpected property: " + name)
    }

    override set(target: Array<T>, name: string, value: NullishType): boolean {
        if (name == ArrayProxy.LENGTH_PROP_NAME) {
            if (value instanceof Number) {
                target.length = value
                return true
            }
        }

        throw new AssertionError("unexpected property: " + name)
    }

    override invoke(target: Array<T>, method: Method, args: NullishType[]): NullishType {
        return super.invoke(target, method, args)
    }

    override get(target: Array<T>, index: int): T {
        return target[index]
    }

    override set(target: Array<T>, index: int, value: T): boolean {
        target[index] = value
        return true
    }
}

abstract class ArrayProxy<T> extends Array<T> {
    public readonly target: Array<T>
    public readonly handler: ArrayProxyHandler<T>

    public static readonly LENGTH_PROP_NAME = "length"

    public static readonly ARRAY_TYPE = Type.of(new Array<Object>()) as ClassType

    public static readonly ARRAY_TYPE_METHODS_COUNT = ArrayProxy.ARRAY_TYPE.getMethodsNum()

    public static readonly ARRAY_TYPE_METHODS: ReadonlyArray<Method> = ArrayProxy.getArrayMethods()

    public static readonly ARRAY_PROXY_DEFINED_METHODS: ReadonlySet<string> = new Set<string>(Array.of<string>(
            "$_get", "$_set", "pop", "concat", "push", "pushECMA", "splice", "toSpliced", "shift",  "unshift"))

    private static getArrayMethods(): Array<Method> {
        const arrayType = ArrayProxy.ARRAY_TYPE

        const arrayMethods = new Array<Method>(ArrayProxy.ARRAY_TYPE_METHODS_COUNT)
        for (let methodIdx = 0; methodIdx < ArrayProxy.ARRAY_TYPE_METHODS_COUNT; methodIdx++) {
            arrayMethods[methodIdx] = arrayType.getMethod(methodIdx)
        }

        return arrayMethods
    }

    private static getArrayMethod(name: string): Method {
        const arrayType = ArrayProxy.ARRAY_TYPE
        for (let methodIdx = 0; methodIdx < ArrayProxy.ARRAY_TYPE_METHODS_COUNT; methodIdx++) {
            const method = ArrayProxy.ARRAY_TYPE_METHODS[methodIdx]
            if (method.getName() == name) {
                return method
            }
        }

        throw new LinkerUnresolvedMethodError("failed to find method: " + name)
    }

    private static getArrayMethod(name: string, signature: Array<Type>): Method {
        const arrayType = ArrayProxy.ARRAY_TYPE
        for (let methodIdx = 0; methodIdx < ArrayProxy.ARRAY_TYPE_METHODS_COUNT; methodIdx++) {
            const method = ArrayProxy.ARRAY_TYPE_METHODS[methodIdx]
            if (method.getName() != name) {
                continue
            }

            const methodType = method.getType()
            const methodParamsCount = methodType.getParametersNum()
            if (methodParamsCount != signature.actualLength) {
                continue
            }

            const signatureMatch = signature.every((paramType: Type, index: number) => {
                const param = methodType.getParameter(index as int)
                return paramType.equals(param.getType())
            })

            if (signatureMatch) {
                return method
            }
        }

        throw new LinkerUnresolvedMethodError("failed to find method: " + name)
    }

    private static getArrayMethod(name: string, firstParam: Type, ...restParams: Type[]): Method {
        const signature = new Array<Type>(firstParam)
        signature.push(...restParams)

        return ArrayProxy.getArrayMethod(name, signature)
    }

    private static readonly POP_METHOD: Method = ArrayProxy.getArrayMethod("pop")
    private static readonly SHIFT_METHOD: Method = ArrayProxy.getArrayMethod("shift")
    private static readonly SPLICE_METHOD_NUM: Method = ArrayProxy.getArrayMethod("splice", Type.of(1.0))
    private static readonly SPLICE_METHOD_INT: Method = ArrayProxy.getArrayMethod("splice", Type.of(1))

    private static readonly SPLICE_METHOD_INT_INT_ARR: Method =
            ArrayProxy.getArrayMethod("splice", Type.of(1), Type.of(1), Type.of(new Object[0]))

    private static readonly SPLICE_METHOD_NUM_OBJ_ARR: Method =
            ArrayProxy.getArrayMethod("splice", Type.of(1.0), Type.of(new Number(0)), Type.of(new Object[0]))

    constructor(target: Array<T>, handler: ArrayProxyHandler<T>) {
        this.target = target
        this.handler = handler
    }

    override $_get(index: int): T {
        return this.handler.get(this.target, index)
    }

    override $_set(index: int, value: T): void {
        const success = this.handler.set(this.target, index, value)
        if (!success) {
            throw new TypeError(`proxy set handler returned false for property '${index}'`)
        }
    }

    // this "wrapper" method is a workaround for #20860
    private handlerGet(target: Array<T>, handler: ProxyHandler<Array<T>>, name: string): NullishType {
        return handler.get(target, name)
    }

    override get length(): number {
        const length = this.handlerGet(this.target, this.handler, ArrayProxy.LENGTH_PROP_NAME)
        if (length instanceof Number) {
            return length.unboxed()
        }

        throw new TypeError("unexpected Array.length value type: " + Type.of(length))
    }

    private handlerSet(target: Array<T>, handler: ProxyHandler<Array<T>>, name: string, value: NullishType): void {
        const success = handler.set(target, name, value)
        if (!success) {
            throw new TypeError(`proxy set handler returned false for propety '"${name}"'`)
        }
    }

    override set length(length: number) {
        this.handlerSet(this.target, this.handler, ArrayProxy.LENGTH_PROP_NAME, length)
    }

    override pop(): T | undefined {
        const result = this.handler.invoke(this.target, ArrayProxy.POP_METHOD, [])
        if (result !== null) {
            return result as T
        }

        throw new AssertionError("Array.pop() returned null")
    }

    override shift(): T | undefined {
        const result = this.handler.invoke(this.target, ArrayProxy.SHIFT_METHOD, [])
        if (result !== null) {
            return result as T
        }

        throw new AssertionError("Array.shift() returned null")
    }

    override concat(...items: ConcatArray<T>[]): Array<T> {
        return this.target.concat(...items)
    }

    override push(...items: T[]): number {
        return this.target.push(...items)
    }

    override pushECMA(...items: T[]): number {
        return this.target.pushECMA(...items)
    }

    override splice(start: number, deleteCount: Number | undefined, ...items: T[]): Array<T> {
        const handler = this.handler
        const result = handler.invoke(this.target, ArrayProxy.SPLICE_METHOD_NUM_OBJ_ARR, [start, deleteCount, items])
        if (result instanceof Array) {
            return result
        }

        throw new AssertionError("Array.splice(int, int, T[]) unexpected result type: " + Type.of(result))
    }

    override splice(start: int, deleteCount: int, ...items: T[]): Array<T> {
        const handler = this.handler
        const result = handler.invoke(this.target, ArrayProxy.SPLICE_METHOD_INT_INT_ARR, [start, deleteCount, items])
        if (result instanceof Array) {
            return result
        }

        throw new AssertionError("Array.splice(int, int, T[]) unexpected result type: " + Type.of(result))
    }

    override splice(start: number): Array<T> {
        const result = this.handler.invoke(this.target, ArrayProxy.SPLICE_METHOD_NUM, [start])
        if (result instanceof Array) {
            return result
        }

        throw new AssertionError("Array.splice(number) unexpected result type: " + Type.of(result))
    }

    override splice(start: int): Array<T> {
        const result = this.handler.invoke(this.target, ArrayProxy.SPLICE_METHOD_INT, [start])
        if (result instanceof Array) {
            return result
        }

        throw new AssertionError("Array.splice(int) unexpected result type: " + Type.of(result))
    }

    override toSpliced(start?: Number, deleteCount?: Number): Array<T> {
        return this.target.toSpliced(start, deleteCount)
    }

    override toSpliced(start: number, deleteCount: number, ...items: T[]): Array<T> {
        return this.target.toSpliced(start, deleteCount, ...items)
    }

    override toSpliced(start: int, deleteCount: int, ...items: T[]): Array<T> {
        return this.target.toSpliced(start, deleteCount, ...items)
    }

    override toSpliced(start: int): Array<T> {
        return this.target.toSpliced(start)
    }

    override unshift(...items: T[]): number {
        return this.target.unshift(...items)
    }
}
