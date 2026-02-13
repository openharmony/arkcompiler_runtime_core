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
 * @file Defines the ReflectProxy for ArkTS
 * @kit ArkTS
 */

package std.core;

/**
 * A namespace that provides reflection and proxy functionality.
 * @namespace reflect
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export namespace reflect {

/**
 * Interface for handling method invocations on a proxy object.
 * Defines methods to manage property access, assignment, and method invocation.
 * @typedef { InvocationHandler }
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export interface InvocationHandler {
    /**
     * Intercepts the getter operation for the each property on the proxy object.
     *
     * @param { Proxy } proxy - The proxy object being accessed.
     * @param { InstanceMethod } method - Intercepted method.
     * @returns { Any } The value of the attribute.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    get(proxy: Proxy, method: InstanceMethod): Any

    /**
     * Intercepts the setter operation of a method on the proxy object.
     *
     * @param { Proxy } proxy - The proxy object being modified.
     * @param { InstanceMethod } method - Intercepted method.
     * @param { Any } value - The value that has been passed to assign through the setter.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    set(proxy: Proxy, method: InstanceMethod, value: Any): void

    /**
     * Intercepts invocation of a method on the proxy object with the provided arguments.
     *
     * @param { Proxy } proxy - The proxy object on which the method is invoked.
     * @param { InstanceMethod } method - Intercepted method.
     * @param { FixedArray<Any> } args - An array of arguments that has been passed to the method.
     * @returns { Any } The result of the method call.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    invoke(proxy: Proxy, method: InstanceMethod, args: FixedArray<Any>): Any
}

/**
 * A base class for creating proxy objects that delegate method calls and property access
 *     to an `InvocationHandler`. Provides static methods for generating proxy classes and
 *     instantiating proxy objects dynamically at runtime.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export class Proxy {
    /**
     * The handler responsible for intercepting method invocations and property access.
     */
    protected handler: InvocationHandler

    /**
     * A prefix used to generate unique names for proxy classes.
     */
    private static readonly PROXY_PREFIX: string = "$Proxy$"

    /**
     * A counter used to ensure unique proxy class names.
     */
    private static uniqueNameCounter: long = -1

    /**
     * Parameters of proxy constructor.
     */
    private static readonly constructorParameters: FixedArray<Class> = [ Class.from<InvocationHandler>() ]

    /**
     * Private default constructor to prevent direct instantiation.
     */
    private constructor() {
    }

    /**
     * Protected constructor that initializes the proxy with a specific invocation handler.
     *
     * @param handler - The `InvocationHandler` to handle method and property operations.
     */
    protected constructor(handler: InvocationHandler) {
        this.handler = handler
    }

    /**
     * Retrieves the invocation handler associated with this proxy.
     * @returns { InvocationHandler } The `InvocationHandler` used by this proxy.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getHandler(): InvocationHandler {
        return this.handler
    }

    /**
     * Creates a new proxy instance that implements the specified interfaces and uses the provided handler.
     *
     * @param { RuntimeLinker } linker - The `RuntimeLinker` used to generate the proxy class at runtime.
     * @param { FixedArray<Class> } interfaces - An array of `Class` objects representing the interfaces to implement.
     * @param { InvocationHandler } handler - The `InvocationHandler` to handle method and property operations.
     * @returns { Proxy } A new proxy instance as an `Proxy`.
     * @throws { Error } - Expected constructor does not exists on given class
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static create(linker: RuntimeLinker, interfaces: FixedArray<Class>, handler: InvocationHandler): Proxy {
        let proxyClass = Proxy.createClass(linker, interfaces)
        // NOTE(kurnevichstanislav): Must be replaced by Type API call: #30649
        let ctor = Proxy.findConstructor(proxyClass, Proxy.constructorParameters)
        if (ctor == undefined) {
            throw new Error("Expected constructor does not exists on given class")
        }
        return ctor.createInstance([handler]) as Proxy
    }

    /**
     * Checks if a class is generated by Proxy.create.
     *
     * @param { Class } klass - The class to check for proxy status.
     * @returns { boolean } `true` if the class is a proxy class, `false` otherwise.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static isProxyClass(klass: Class): boolean {
        return klass.isSubtypeOf(Class.from<Proxy>()) && Proxy.hasProxyFlag(klass)
    }

    /**
     * Checks whether the specified class is marked as a proxy class.
     *
     * @param { Class } klass - The class to check for proxy status.
     * @returns { boolean } `true` if the class contains the proxy flag, `false` otherwise.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    private static native hasProxyFlag(klass: Class): boolean

    private static createUniqueName(): string {
        return Proxy.PROXY_PREFIX.concat((++Proxy.uniqueNameCounter).toString())
    }

    /**
     * Creates a new proxy `Class` that implements the specified interfaces and extends `Proxy` class.
     *
     * @returns A `Class` object representing the dynamically generated proxy class.
     */
    private static createClass(linker: RuntimeLinker, interfaces: FixedArray<Class>): Class {
        let proxyName: string = Proxy.createUniqueName()
        return Proxy.generateProxy(linker, proxyName, interfaces)
    }

    private static native generateProxy(
        linker: RuntimeLinker,
        proxyName: string,
        interfaces: FixedArray<Class>,
    ): Class

    // Called by runtime in the compiled entrypoint.
    private static invoke(proxy: Proxy, method: InstanceMethod, args: FixedArray<Any>): Any {
        return proxy.handler.invoke(proxy, method, args)
    }

    // Called by runtime in the compiled entrypoint.
    private static invokeGet(proxy: Proxy, method: InstanceMethod): Any {
        return proxy.handler.get(proxy, method)
    }

    // Called by runtime in the compiled entrypoint.
    private static invokeSet(proxy: Proxy, method: InstanceMethod, value: Any): void {
        proxy.handler.set(proxy, method, value)
    }

    // NOTE(kurnevichstanislav): Must be replaced by Type API call: #30649
    private static findConstructor(klass: Class, params: FixedArray<Class>): reflect.Constructor | undefined {
        let numParams = params.length
        let constructors = klass.getConstructors()
        let numCtors = constructors.length
        for (let idx = 0; idx < numCtors; ++idx) {
            if (constructors[idx].getParametersNum() == numParams) {
                if (Proxy.compareTypesUnchecked(params, constructors[idx].getParameterTypes())) {
                    return constructors[idx]
                }
            }
        }
        return undefined
    }

    // NOTE(kurnevichstanislav): Must be replaced by Type API call: #30649
    private static compareTypesUnchecked(lhs: FixedArray<Class>, rhs: FixedArray<Class>): boolean {
        for (let idx = 0; idx < lhs.length; ++idx) {
            if (lhs[idx] != rhs[idx]) {
                return false
            }
        }
        return true
    }
} // export class Proxy

} // export namespace reflect
