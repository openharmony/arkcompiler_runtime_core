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
import st from '@ohos.lang.interop';

export declare let a: Array<string>;
export declare const b: Set<number>;
export declare let c: Map<string, any>;
export declare const d: st.Array<string>;
export declare const e: st.Set<number>;
export declare let f: st.Map<string, any>;
export declare function func1(p: st.Array<string>): Array<string>;
export declare function func2(p: st.Set<number>): Set<number>;
export declare function func3(p: st.Map<string, any>): Map<string, any>;
export declare function func4(p: Array<string>): st.Array<string>;
export declare function func5(p: Set<number>): st.Set<number>;
export declare function func6(p: Map<string, any>): st.Map<string, any>;
export declare function noConvertForRestParams(...p: Array<string>): void;
export declare let composedVar1: Array<st.Map<string, st.Set<number>>>;
export declare let composedVar2: st.Map<string, Array<st.Set<number>>>;
export declare let composedVar3: st.Array<st.Map<string, st.Set<number>>>;
export declare let composedVar4: Map<string, st.Array<st.Set<number>>>;
export declare let arrA: string[]
export declare let arrB: string[][]
export declare let arrC: (string | number)[]
export declare let arrD: string[] | number[]
export declare let arrE: Array<string | number>[]
export declare class ESClass {
    member1: Array<string>;
    member2: Set<number>;
    member3: Map<string, any>;
}
export declare class STClass {
    member1: st.Array<string>;
    member2: st.Set<number>;
    member3: st.Map<string, any>;
}
