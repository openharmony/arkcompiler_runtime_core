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

/**
 * Interface for handling method invocations on a proxy object.
 * Defines methods to manage property access, assignment, and method invocation.
 */
export interface InvocationHandler {
    /**
     * Intercepts the getter operation for the each property on the proxy object.
     *
     * @param proxy - The proxy object being accessed.
     * @param method - Intercepted method.
     */
    get(proxy: Proxy, method: InstanceMethod): Any

    /**
     * Intercepts the setter operation of a method on the proxy object.
     *
     * @param proxy - The proxy object being modified.
     * @param method - Intercepted method.
     * @param value - The value that has been passed to assign through the setter.
     */
    set(proxy: Proxy, method: InstanceMethod, value: Any): void

    /**
     * Intercepts invocation of a method on the proxy object with the provided arguments.
     *
     * @param proxy - The proxy object on which the method is invoked.
     * @param method - Intercepted method.
     * @param args - An array of arguments that has been passed to the method.
     */
    invoke(proxy: Proxy, method: InstanceMethod, args: FixedArray<Any>): Any
}

/**
 * A base class for creating proxy objects that delegate method calls and property access
 * to an `InvocationHandler`. Provides static methods for generating proxy classes and
 * instantiating proxy objects dynamically at runtime.
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
    private static readonly constructorParameters: FixedArray<Class> = [ (Type.from<InvocationHandler>() as InterfaceType).getClass() ]

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
     *
     * @returns The `InvocationHandler` used by this proxy.
     */
    public getHandler(): InvocationHandler {
        return this.handler
    }

    /**
     * Creates a new proxy instance that implements the specified interfaces and uses the provided handler.
     *
     * @param linker - The `RuntimeLinker` used to generate the proxy class at runtime.
     * @param interfaces - An array of `Class` objects representing the interfaces to implement.
     * @param handler - The `InvocationHandler` to handle method and property operations.
     * @returns A new proxy instance as an `Proxy`.
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

    private static createUniqueName(interfaces: FixedArray<Class>): string {
        return Proxy.PROXY_PREFIX.concat((++Proxy.uniqueNameCounter).toString())
    }

    /**
     * Creates a new proxy `Class` that implements the specified interfaces and extends `Proxy` class.
     *
     * @returns A `Class` object representing the dynamically generated proxy class.
     */
    private static createClass(linker: RuntimeLinker, interfaces: FixedArray<Class>): Class {
        let methods: Array<reflect.InstanceMethod> = Proxy.getMethods(interfaces)
        let proxyName: string = Proxy.createUniqueName(interfaces)
        return Proxy.generateProxy(linker, proxyName, interfaces, methods)
    }

    private static native generateProxy(
        linker: RuntimeLinker,
        proxyName: string,
        interfaces: FixedArray<Class>,
        methods: Array<reflect.InstanceMethod>
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

    private static getMethods(ifaces: FixedArray<Class>): Array<reflect.InstanceMethod> {
        let instanceMethods = Array<reflect.InstanceMethod>()
        // NOTE(kurnevichstanislav): should be implemented more complex analisys of signature
        // with respect to methods overloading #29358.
        let namesSet = new Set<string>()

        for(let idx = 0; idx < ifaces.length; ++idx) {
            let methodsSet = Proxy.getInstanceMethodsForInterface(ifaces[idx])
            for (let method of methodsSet) {
                if (!namesSet.has(method.getName())) {
                    namesSet.add(method.getName())
                    instanceMethods.pushOne(method)
                } else {
                    throw new Error("Proxy does not support overloads and overrides.")
                }
            }
        }
        return instanceMethods
    }

    // NOTE(kurnevichstanislav): Must be replaced by Type API call: #30649
    private static findConstructor(klass: Class, params: FixedArray<Class>): reflect.Constructor | undefined {
        let numParams = params.length
        let constructors = klass.getConstructors()
        let numCtors = constructors.length
        for (let idx = 0; idx < numCtors; ++idx) {
            if (constructors[idx].getParametersNum() == numParams) {
                if (Proxy.compareTypesUnchecked(params, constructors[idx].getParametersTypes())) {
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

    private static getInstanceMethodsForInterface(iface: Class): Set<reflect.InstanceMethod> {
        return Proxy.getInstanceMethodsForInterfaceRecursive(iface, new Set<Class>())
    }

    // NOTE(kurnevichstanisalv): Support overloads and override-compatible
    private static getInstanceMethodsForInterfaceRecursive(iface: Class, visited: Set<Class>): Set<reflect.InstanceMethod> {
        let methods = new Set<reflect.InstanceMethod>()
        if (!visited.add(iface)) {
            return methods
        }

        // First, collect all methods from parent interfaces
        let superIfaces = iface.getInterfaces()
        let inheritedMethods = new Set<reflect.InstanceMethod>()

        for (let superIntf of superIfaces) {
            let superMethods = Proxy.getInstanceMethodsForInterfaceRecursive(superIntf, visited)
            for (let superMethod of superMethods) {
                inheritedMethods.add(superMethod)
            }
        }

        // Add direct methods first (these have highest priority)
        let directMethods = iface.getInstanceMethods();
        for (let directMethod of directMethods) {
            methods.add(directMethod)
        }

        // Add inherited methods that aren't overridden by direct methods
        for (let inheritedMethod of inheritedMethods) {
            let overridden = false

            // Check if any direct method overrides this specific inherited method
            for (let directMethod of directMethods) {
                if (Proxy.hasSameSignature(inheritedMethod, directMethod)) {
                    overridden = true
                    break
                }
            }

            // Also check if we already have a method with the same signature from another parent interface
            if (!overridden) {
                let alreadyExists = false
                for (let existingMethod of methods) {
                    if (Proxy.hasSameSignature(inheritedMethod, existingMethod)) {
                        alreadyExists = true
                        break
                    }
                }
                if (!alreadyExists) {
                    methods.add(inheritedMethod)
                }
            }
        }

        return methods
    }

    private static hasSameSignature(method1: reflect.InstanceMethod, method2: reflect.InstanceMethod): boolean {
        return method1.getName() === method2.getName() &&
               method1.getParametersNum() == method2.getParametersNum() &&
               Proxy.compareTypesUnchecked(method1.getParametersTypes(), method2.getParametersTypes())
    }
} // export class Proxy

} // export namespace reflect
