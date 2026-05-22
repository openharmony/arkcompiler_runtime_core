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
// ==================== Top-level Function Cases ====================

// Case 1: Top-level function overload merge
export declare function topFunc(a: string): string;
export declare function topFunc(a: string): number;

// Case 2: Top-level function different params → NOT merged
export declare function diffFunc(a: string): void;
export declare function diffFunc(a: number): void;

// Case 3: Top-level function three-way merge
export declare function threeFunc(a: number): string;
export declare function threeFunc(a: number): number;
export declare function threeFunc(a: number): boolean;

// Case 4: Top-level function with generic merge
export declare function genFunc<T>(a: T): string;
export declare function genFunc<U>(a: U): number;
export declare function genFunc(a: any): number;

// Case 5: Namespace function overload merge
export declare namespace MyNS {
  function nsFunc(a: string): string;
  function nsFunc(a: string): number;
}

// Case 6: Top-level function with optional param
export declare function optFunc(a: string, b: number): string;
export declare function optFunc(a: string, b?: number): string;

// Case 7: Top-level function different param names → param0
export declare function namedFunc(x: string): string;
export declare function namedFunc(y: string): number;

// Case 8: Top-level function with rest params — same type
export declare function restFunc(...args: number[]): string;
export declare function restFunc(...args: number[]): number;

// Case 9: Single function — no merge needed
export declare function singleFunc(a: string): void;

// ==================== Assembly Type Function Cases ====================

// Case 10: Type alias param merge — assembly types match → merge
type FnAlias = string;
export declare function aliasFunc(a: string): string;
export declare function aliasFunc(a: FnAlias): number;

// Case 11: Literal type param → primitive assembly type → merge
export declare function literalFunc(a: "hello"): string;
export declare function literalFunc(a: string): number;

// Case 12: Rest params different types → Any[]
export declare function restDiffFunc(...args: number[]): string;
export declare function restDiffFunc(...args: string[]): number;

// Case 13: Rest params different names → ...args
export declare function restNameFunc(...items: number[]): string;
export declare function restNameFunc(...values: number[]): number;

// Case 14: Rest vs non-rest same assembly → ...args: Any[]
export declare function restVsNonFunc(...args: number[]): string;
export declare function restVsNonFunc(p: string[]): number;

// Case 15: Array param merge → string[] | number[]
export declare function arrayFunc(a: string[]): string;
export declare function arrayFunc(a: number[]): number;

// Case 16: Tuple param same count → merge with union
export declare function tupleFunc(a: [string, number]): string;
export declare function tupleFunc(a: [number, string]): number;

// Case 17: Generic class param → Set<string> | Set<number>
export declare function genericFunc(a: Set<string>): string;
export declare function genericFunc(a: Set<number>): number;

// Case 18: Return type — void + value → undefined | value
export declare function retVoidFunc(a: string): void;
export declare function retVoidFunc(a: string): number;

// Case 19: Return type — dedup identical
export declare function retDedupFunc(a: string): string;
export declare function retDedupFunc(a: string): string;

// Case 20: Return type — generic T + any → T | any
export declare function retGenAnyFunc<T>(a: string): T;
export declare function retGenAnyFunc(a: string): any;

// Case 21: Namespace function with type alias merge
type NsAlias = number;
export declare namespace MyNS2 {
  function nsAliasFunc(a: number): string;
  function nsAliasFunc(a: NsAlias): number;
}

// Case 22: Rest param with tuple → Any[]
export declare function restTupleFunc(...args: [string, number]): string;
export declare function restTupleFunc(...args: [number, string]): number;

// Case 23: Multiple params — literal + optional combo
export declare function comboFunc(a: "yes", b: number): string;
export declare function comboFunc(a: string, b?: number): number;
