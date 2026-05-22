'use static'
/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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
export declare let cb: (x: number) => number;
export type ComplexNormalFunc = (arg1: number, arg2: boolean) => string[];
export declare let lambdaFunctionGeneric: (a: Any) => Any;
export type GenericFunc = (arg1: Any, arg2: Any) => Array<Any>;
export type ConstrainedGenericFunc = (arg: Any) => Any;
export type NestedGenericFunc = (fn: (u: Any) => Any) => Any;
export type DefaultGenericFunc = (arg?: Any) => Any;
export declare class TestClass {
    genericFunc: (value: Any) => Any;
    normalFunc: (value: number) => number;
}
export declare interface TestInterface {
    genericMethod: (data: Any) => Any;
    normalMethod: (data: string) => string;
}
