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

// Case 1: Basic constructor overload merge — same param types
export class BasicMerge {
    constructor(a: string);
    constructor(a: string);
    constructor(a: string) {}
}

// Case 2: Constructor overloads with different param types — NOT merged
export declare class DiffParamTypes {
    constructor(a: string);
    constructor(a: number);
    constructor(a: string | number);
}

// Case 3: Three-way merge — all identical signatures
export class ThreeWayMerge {
    constructor(a: string, b: number);
    constructor(a: string, b: number);
    constructor(a: string, b: number);
    constructor(a: string, b: number) {}
}

// Case 4: Optional param merge — required + optional → optional
export class OptionalParamMerge {
    constructor(a: string, b: number);
    constructor(a: string, b?: number);
    constructor(a: string, b?: number) {}
}

// Case 5: Different param count — NOT in same cluster
export class DiffParamCount {
    constructor(a: string);
    constructor(a: string, b: number);
    constructor(a: string, b?: number) {}
}

// Case 6: Multiple param counts with duplicates in each group
export class MultiCountDuplicates {
    constructor(a: string);
    constructor(a: string);
    constructor(a: number, b: boolean);
    constructor(a: number, b: boolean);
    constructor(a: string | number, b?: boolean) {}
}

// Case 7: Constructor with 'any' type — wildcard merge
export class AnyTypeMerge {
    constructor(a: string);
    constructor(a: any);
    constructor(a: any) {}
}

// Case 8: Constructor in generic class
export class GenericClass<T> {
    constructor(a: T);
    constructor(a: T);
    constructor(a: T) {}
}

// Case 9: Constructor with array params — same element type
export class ArrayParamMerge {
    constructor(items: string[]);
    constructor(items: string[]);
    constructor(items: string[]) {}
}

// Case 10: Constructor with tuple params — same tuple length → merged
export class TupleParamMerge {
    constructor(pair: [string, number]);
    constructor(pair: [number, string]);
    constructor(pair: [string, number] | [number, string]) {}
}

// Case 11: Class with both constructors and methods — only ctors merge
export class MixedMembers {
    constructor(a: string);
    constructor(a: string);
    constructor(a: string) {}
    greet(name: string): void { }
    farewell(name: string): void { }
}

// Case 12: Original Dummy case — different arity overloads
export class Dummy {
    constructor(param0: string);
    constructor(param0: number, param1?: boolean);
    constructor(param0: number | string, param1?: boolean) {}
}

// Case 13: Original Test case — complex multi-arity overloads
export class Test {
    constructor();
    constructor(async?: boolean);
    constructor(syncFlags: number, waitTime: number);
    constructor(param0: string, param1: number);
    constructor(param0: string, param1: number, param2: string);
    constructor(syncFlags?: number, waitTime?: number);
    constructor(param0?: boolean | number | string, param1?: number, param2?: string) {
    }
}

// ==================== Assembly Type Constructor Cases ====================

// Case 14: Type alias param merge — assembly types match
type CtorAlias = string;
export class TypeAliasCtor {
    constructor(a: string);
    constructor(a: CtorAlias);
    constructor(a: string) {}
}

// Case 15: Literal type param → primitive → merge
export class LiteralCtor {
    constructor(a: "hello");
    constructor(a: string);
    constructor(a: string) {}
}

// Case 16: Array param different element types → same assembly → merge
export class ArrayDiffCtor {
    constructor(a: string[]);
    constructor(a: number[]);
    constructor(a: string[] | number[]) {}
}

// Case 17: Generic class param ignores type args → merge
export class GenericParamCtor {
    constructor(a: Set<string>);
    constructor(a: Set<number>);
    constructor(a: Set<string> | Set<number>) {}
}

// Case 18: Rest param — same type
export class RestSameCtor {
    constructor(...args: number[]);
    constructor(...args: number[]);
    constructor(...args: number[]) {}
}

// Case 19: Rest param — different types → Any[]
export class RestDiffCtor {
    constructor(...args: number[]);
    constructor(...args: string[]);
    constructor(...args: any[]) {}
}

// Case 20: Rest param — different names → ...args
export class RestNameCtor {
    constructor(...items: number[]);
    constructor(...values: number[]);
    constructor(...args: number[]) {}
}

// Case 21: Rest vs non-rest same assembly → ...args: Any[]
export class RestVsNonRestCtor {
    constructor(...args: number[]);
    constructor(p: string[]);
    constructor(...args: any[]) {}
}

// Case 22: Tuple param same count → merge
export class TupleSameCountCtor {
    constructor(a: [string, number]);
    constructor(a: [number, string]);
    constructor(a: [string, number] | [number, string]) {}
}

// Case 23: Any + generic wildcard merge
export class AnyGenericCtor<T> {
    constructor(a: T);
    constructor(a: any);
    constructor(a: any) {}
}

// Case 24: Rest param with tuple → Any[]
export class RestTupleCtor {
    constructor(...args: [string, number]);
    constructor(...args: [number, string]);
    constructor(...args: any[]) {}
}