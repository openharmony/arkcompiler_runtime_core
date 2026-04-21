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
// ==================== Assembly Type Cases ====================
// Case 38: Type alias — assembly type is the original type, so they merge
type MyString = string;
// Case 39: Type alias + original — merged param keeps both as X | Xa union
type NumAlias = number;
// Case 49: Return type merge — type alias kept distinct from original
type RetAlias = string;
// Case 1: Original — different arity overloads (NOT merged)
export declare class MathUtils2 {
    add(num: number): number;
    add(num1: number, num2: number): number;
}
// Case 2: Basic duplicate merge — same name, same param types, different return
export declare class BasicMerge {
    foo(a: string): string | number;
}
// Case 3: void return type → undefined in union
export declare class VoidReturn {
    bar(a: number): undefined | string;
}
// Case 4: Optional parameter merging
export declare class OptionalParam {
    baz(a?: string): number;
}
// Case 5: any treated as wildcard
export declare class AnyEquivalence {
    qux(a: Any): string | number;
}
// Case 6: Different param count → NOT merged
export declare class DifferentParamCount {
    foo(a: string): void;
    foo(a: string, b: number): void;
}
// Case 7: Different param types → NOT merged
export declare class DifferentParamTypes {
    foo(a: string): void;
    foo(a: number): void;
}
// Case 8: Three-way merge
export declare class ThreeWayMerge {
    foo(a: string): string | number | boolean;
}
// Case 9: Union return type flattened
export declare class UnionReturnFlat {
    foo(a: string): string | number | boolean;
}
// Case 10: void in union return
export declare class VoidInUnionReturn {
    foo(a: string): Any | number;
}
// Case 11: Duplicate return types deduped
export declare class DuplicateReturnDedup {
    foo(a: string): string;
}
// Case 12: Multiple parameters with mixed optionality
export declare class MultiParamOptional {
    foo(a: string, b?: number): undefined;
}
// Case 13: Other members preserved alongside merged methods
export declare class MixedMembers {
    x: number;
    foo(a: string): string | number;
    readonly y: boolean;
}
// Case 14: Multiple different method names merged independently
export declare class MultipleNames {
    foo(a: string): string | number;
    bar(a: number): boolean | string;
}
// Case 15: Methods with generic type parameters
export declare class GenericMethods {
    foo<T, U>(a: T | U): T | string;
}
// Case 16: any vs T — wildcard merge, T | Any
export declare class AnyVsTypeParam {
    foo<T>(a: T | Any): string | number;
}
// Case 17: Generic T vs concrete type → NOT duplicates
export declare class GenericVsConcrete {
    foo<T>(a: T): void;
    foo(a: string): void;
}
// Case 18: Complex types — deeply nested generics (same generic class = merged)
export declare class ComplexTypes {
    foo(a: Array<string>): Map<string, number> | Set<string>;
}
// Case 19: Rest parameters — same type
export declare class RestParam {
    foo(...args: string[]): undefined | number;
}
// Case 20: Array type duplicate detection (same assembly type Array)
export declare class ArrayMerge {
    foo(a: number[] | string[]): string | number;
}
// Case 21: Tuple type duplicate detection, to Array<Any>
export declare class TupleMerge {
    foo(a: Array<Any>): string | number;
}
// Case 22: Tuple type different lengths → NOT duplicates
export declare class TupleDiffLength {
    foo(a: Array<Any>): string | number;
}
// Case 23: Generic class duplicate detection (same base type = merged)
export declare class GenericClassMerge {
    foo(a: Array<number> | Array<string>): string | number;
}
// Case 24: Different generic classes → NOT duplicates
export declare class GenericClassDiff {
    foo(a: Array<number>): string;
    foo(a: Map<string, number>): number;
}
// Case 25: Parameter name differs across overloads → param0
export declare class DiffParamNames {
    greet(param0: string): string | number;
}
// Case 26: Single method — no merge needed
export declare class SingleMethod {
    foo(a: string): void;
}
// Case 27: No methods — only properties
export declare class NoMethods {
    x: number;
    y: string;
}
// Case 28: Instance method in generic class — class generic T + method generic U
export declare class GenericClass<T> {
    method<U>(a: T | U): T | string;
}
// ==================== Static Method Cases ====================
// Case 29: Original static — different arity overloads (NOT merged)
export declare class MathUtils {
    static add(num: number): number;
    static add(num1: number, num2: number): number;
}
// Case 30: Static method overloads merged
export declare class StaticOverload {
    static doWork(a: string): string | number;
}
// Case 31: Static methods with different param types → NOT merged
export declare class StaticDiffParams {
    static run(a: string): void;
    static run(a: number): void;
}
// Case 32: Static three-way merge
export declare class StaticThreeWay {
    static compute(a: number): string | number | boolean;
}
// Case 33: Static + instance same name, both have overloads — merged independently
export declare class StaticAndInstanceOverloads {
    process(a: string): string | number;
    static process(a: string): boolean | undefined;
}
// Case 34: Static method with optional param merge
export declare class StaticOptional {
    static init(a: string, b?: number): undefined;
}
// Case 35: Static method with mixed members preserved
export declare class StaticMixedMembers {
    static count: number;
    static reset(a: string): undefined | boolean;
    getName(): string;
}
// Case 36: Static vs instance same name → NOT merged (different clusters)
export declare class StaticVsInstance {
    foo(a: string): void;
    static foo(a: string): number;
}
// Case 37: Static parameter name differs across overloads → param0
export declare class StaticDiffParamNames {
    static greet(param0: string): string | number;
}
export declare class TypeAliasMerge {
    foo(a: string | MyString): string | number;
}
export declare class TypeAliasParamUnion {
    bar(a: number | NumAlias): undefined;
}
// Case 40: Literal type → primitive assembly type
// "hello" → string, 42 → number, true → boolean, so all merge with corresponding primitive
export declare class LiteralTypeMerge {
    foo(a: "hello" | string): string | number;
    bar(a: number): string | number;
    baz(a: boolean): string | number;
}
// Case 41: Generic class ignores type args for assembly type → merge
export declare class GenericIgnoreArgs {
    foo(a: Set<string> | Set<number>): string | number;
}
// Case 42: readonly preserved in assembly type
// readonly number[] and number[] have different assembly types → NOT merged
export declare class ReadonlyNotMerged {
    foo(a: Any): string;
    foo(a: number[]): number;
}
// Case 43: Literal types: multiple distinct literals of same primitive → merge
export declare class MultiLiteral {
    foo(a: "hello" | "world"): string | number;
}
// Case 44: Rest parameter — different types → Any[]
export declare class RestDiffTypes {
    foo(...args: Array<Any>): string | number;
}
// Case 45: Rest parameter — different names → ...args
export declare class RestDiffNames {
    foo(...args: number[]): string | number;
}
// Case 46: Rest vs non-rest same assembly type → merged to rest with Any[]
export declare class RestVsNonRest {
    foo(...args: Array<Any>): string | number;
}
// Case 47: Tuple with different element types but same count → merged
export declare class TupleSameCount {
    foo(a: Array<Any>): string | number;
}
export declare class ReturnTypeAlias {
    foo(a: number): string | RetAlias;
}
// Case 50: Return type merge — void + generic T → undefined | T
export declare class ReturnVoidGeneric {
    foo<T, U>(a: string): undefined | U;
}
// Case 51: Return type merge — generic T + Any → T | Any
export declare class ReturnGenericAny {
    foo<T>(a: string): T | Any;
}
// Case 52: Merge type params — class generics preserved alongside method generics
export declare class ClassGenericMerge<T> {
    foo<U>(a: T | U): T | U;
}
// Case 53: Array param merge — string[] and number[] → string[] | number[]
export declare class ArrayParamUnion {
    foo(a: string[] | number[]): undefined;
}
// Case 54: Generic class param merge — Set<string> and Set<number> → Set<string> | Set<number>
export declare class GenericParamUnion {
    foo(a: Set<string> | Set<number>): undefined;
}
// Case 55: Tuple param merge — [string, number] and [number, string] → union
export declare class TupleParamUnion {
    foo(a: Array<Any>): undefined;
}
// Case 56: Multiple params with mixed features
// param0: literal + primitive → merge; param1: optional merge
export declare class MixedParamFeatures {
    foo(a: "yes" | string, b?: number): string | number;
}
// Case 57: Rest param merge — tuple types → Any[]
export declare class RestTupleMerge {
    foo(...args: Array<Any>): string | number;
}
// Case 58: Class generic T vs method generic U merge
// Should preserve c1 (T from class) and t1 (U from method)
export declare class ClassVsMethodGeneric<T> {
    foo<U>(a: T | U | Any): T | string | number;
}
// Case 48: Interface with method overloads
export declare interface InterfaceMerge {
    foo(a: string): string | number;
    bar(a: number): undefined | boolean;
}
