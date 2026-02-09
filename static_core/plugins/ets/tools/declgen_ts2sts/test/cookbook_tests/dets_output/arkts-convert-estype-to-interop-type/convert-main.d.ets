'use static'
/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
import es from "@ohos.lang.interop";
export declare let a: es.Array<string>;
export declare const b: es.Set<number>;
export declare let c: es.Map<string, Any>;
export declare const d: Array<string>;
export declare const e: Set<number>;
export declare let f: Map<string, Any>;
export declare function func1(p: Array<string>): es.Array<string>;
export declare function func2(p: Set<number>): es.Set<number>;
export declare function func3(p: Map<string, Any>): es.Map<string, Any>;
export declare function func4(p: es.Array<string>): Array<string>;
export declare function func5(p: es.Set<number>): Set<number>;
export declare function func6(p: es.Map<string, Any>): Map<string, Any>;
export declare function noConvertForRestParams(...p: string[]): void;
export declare let composedVar1: es.Array<Map<string, Set<number>>>;
export declare let composedVar2: Map<string, es.Array<Set<number>>>;
export declare let composedVar3: Array<Map<string, Set<number>>>;
export declare let composedVar4: es.Map<string, Array<Set<number>>>;
export declare let arrA: es.Array<string>;
export declare let arrB: es.Array<es.Array<string>>;
export declare let arrC: es.Array<(string | number)>;
export declare let arrD: es.Array<string> | es.Array<number>;
export declare let arrE: es.Array<es.Array<string | number>>;
export declare class ESClass {
    member1: es.Array<string>;
    member2: es.Set<number>;
    member3: es.Map<string, Any>;
}
export declare class STClass {
    member1: Array<string>;
    member2: Set<number>;
    member3: Map<string, Any>;
}
