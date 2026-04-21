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
// ==================== Top-level Function Cases ====================
// Case 1: Top-level function overload merge
export declare function topFunc(a: string): string | number;
// Case 2: Top-level function different params → NOT merged
export declare function diffFunc(a: string): void;
export declare function diffFunc(a: number): void;
// Case 3: Top-level function three-way merge
export declare function threeFunc(a: number): string | number | boolean;
// Case 4: Top-level function with generic merge
export declare function genFunc<T, U>(a: T | U | Any): string | number;
// Case 5: Namespace function overload merge
export declare namespace MyNS {
    export function nsFunc(a: string): string | number;
}
// Case 6: Top-level function with optional param
export declare function optFunc(a: string, b?: number): string;
// Case 7: Top-level function different param names → param0
export declare function namedFunc(param0: string): string | number;
// Case 8: Top-level function with rest params — same type
export declare function restFunc(...args: number[]): string | number;
// Case 9: Single function — no merge needed
export declare function singleFunc(a: string): void;
// ==================== Assembly Type Function Cases ====================
// Case 10: Type alias param merge — assembly types match → merge
type FnAlias = string;
export declare function aliasFunc(a: string | FnAlias): string | number;
// Case 11: Literal type param → primitive assembly type → merge
export declare function literalFunc(a: "hello" | string): string | number;
// Case 12: Rest params different types → Any[]
export declare function restDiffFunc(...args: Array<Any>): string | number;
// Case 13: Rest params different names → ...args
export declare function restNameFunc(...args: number[]): string | number;
// Case 14: Rest vs non-rest same assembly → ...args: Any[]
export declare function restVsNonFunc(...args: Array<Any>): string | number;
// Case 15: Array param merge → string[] | number[]
export declare function arrayFunc(a: string[] | number[]): string | number;
// Case 16: Tuple param same count → merge into Array<Any>
export declare function tupleFunc(a: Array<Any>): string | number;
// Case 17: Generic class param → Set<string> | Set<number>
export declare function genericFunc(a: Set<string> | Set<number>): string | number;
// Case 18: Return type — void + value → undefined | value
export declare function retVoidFunc(a: string): undefined | number;
// Case 19: Return type — dedup identical
export declare function retDedupFunc(a: string): string;
// Case 20: Return type — generic T + any → T | any
export declare function retGenAnyFunc<T>(a: string): T | Any;
// Case 21: Namespace function with type alias merge
type NsAlias = number;
export declare namespace MyNS2 {
    export function nsAliasFunc(a: number | NsAlias): string | number;
}
// Case 22: Rest param with tuple → Any[]
export declare function restTupleFunc(...args: Array<Any>): string | number;
// Case 23: Multiple params — literal + optional combo
export declare function comboFunc(a: "yes" | string, b?: number): string | number;
