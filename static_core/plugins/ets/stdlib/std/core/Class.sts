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

    private constructor() { throw new Error("Class constructor called") }
    private native getNameInternal(): string;

    public getName(): string {
        return this.name ?? this.getNameInternal()
    }

    public getSuper(): Class | undefined {
        return this.superClass
    }

    public native getLinker(): RuntimeLinker

    /**
     * Invoke class initializer once if class is not initialized
     */
    public native initialize(): void

    public static native of(obj: Object): Class

    /**
     * Get class of caller method
     */
    public static native current(): Class

    /**
     * Get class of caller method which is calling this method
     */
    public static native ofCaller(): Class

    public native createInstance(): Object

    /**
     * Get internal string descriptor
     */
    public native getDescriptor(): string
}
