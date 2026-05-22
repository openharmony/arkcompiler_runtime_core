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
// ==================== Assembly Type Constructor Cases ====================
// Case 14: Type alias param merge — assembly types match
type CtorAlias = string;
// Case 1: Basic constructor overload merge — same param types
export declare class BasicMerge {
    constructor(a: string);
}
// Case 2: Constructor overloads with different param types — NOT merged
export declare class DiffParamTypes {
    constructor(a: string);
    constructor(a: number);
    constructor(a: string | number);
}
// Case 3: Three-way merge — all identical signatures
export declare class ThreeWayMerge {
    constructor(a: string, b: number);
}
// Case 4: Optional param merge — required + optional → optional
export declare class OptionalParamMerge {
    constructor(a: string, b?: number);
}
// Case 5: Different param count — NOT in same cluster
export declare class DiffParamCount {
    constructor(a: string);
    constructor(a: string, b: number);
}
// Case 6: Multiple param counts with duplicates in each group
export declare class MultiCountDuplicates {
    constructor(a: string);
    constructor(a: number, b: boolean);
}
// Case 7: Constructor with 'any' type — wildcard merge
export declare class AnyTypeMerge {
    constructor(a: string);
    constructor(a: Any);
}
// Case 8: Constructor in generic class
export declare class GenericClass<T> {
    constructor(a: T);
}
// Case 9: Constructor with array params — same element type
export declare class ArrayParamMerge {
    constructor(items: string[]);
}
// Case 10: Constructor with tuple params — same tuple length → merged
export declare class TupleParamMerge {
    constructor(pair: Array<Any>);
}
// Case 11: Class with both constructors and methods — only ctors merge
export declare class MixedMembers {
    constructor(a: string);
    greet(name: string): void;
    farewell(name: string): void;
}
// Case 12: Original Dummy case — different arity overloads
export declare class Dummy {
    constructor(param0: string);
    constructor(param0: number, param1?: boolean);
}
// Case 13: Original Test case — complex multi-arity overloads
export declare class Test {
    constructor();
    constructor(_async?: boolean);
    constructor(syncFlags?: number, waitTime?: number);
    constructor(param0: string, param1: number);
    constructor(param0: string, param1: number, param2: string);
}
export declare class TypeAliasCtor {
    constructor(a: string | CtorAlias);
}
// Case 15: Literal type param → primitive → merge
export declare class LiteralCtor {
    constructor(a: "hello" | string);
}
// Case 16: Array param different element types → same assembly → merge
export declare class ArrayDiffCtor {
    constructor(a: string[] | number[]);
}
// Case 17: Generic class param ignores type args → merge
export declare class GenericParamCtor {
    constructor(a: Set<string> | Set<number>);
}
// Case 18: Rest param — same type
export declare class RestSameCtor {
    constructor(...args: number[]);
}
// Case 19: Rest param — different types → Array<Any>
export declare class RestDiffCtor {
    constructor(...args: Array<Any>);
}
// Case 20: Rest param — different names → ...args
export declare class RestNameCtor {
    constructor(...args: number[]);
}
// Case 21: Rest vs non-rest same assembly → ...args: Array<Any>
export declare class RestVsNonRestCtor {
    constructor(...args: Array<Any>);
}
// Case 22: Tuple param same count → merge
export declare class TupleSameCountCtor {
    constructor(a: Array<Any>);
}
// Case 23: Any + generic wildcard merge
export declare class AnyGenericCtor<T> {
    constructor(a: T | Any);
}
// Case 24: Rest param with tuple → Array<Any>
export declare class RestTupleCtor {
    constructor(...args: Array<Any>);
}
