/**
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

export class StandardClass {
    public name: string = 'Alice';
    public age: int = 30;
}

export class CustomClass implements JsonSerializable {
    public toJSON(): string {
        return '{"custom":"json_works"}';
    }
}

export class BaseClass implements JsonSerializable {
    public toJSON(): string {
        return '{"base":true}';
    }
}

export class DerivedClass extends BaseClass { }

export class OverridingClass extends BaseClass {
    public override toJSON(): string {
        return '{"overridden":true}';
    }
}

export class NestedClass implements JsonSerializable {
    public toJSON(): string {
        return '{"nested":{"deep":true},"array":[1,2,3]}';
    }
}

export function createStandard(): StandardClass {
    return new StandardClass();
}

export function createCustom(): CustomClass {
    return new CustomClass();
}

export function createBase(): BaseClass {
    return new BaseClass();
}

export function createDerived(): DerivedClass {
    return new DerivedClass();
}

export function createOverriding(): OverridingClass {
    return new OverridingClass();
}

export function createNested(): NestedClass {
    return new NestedClass();
}

export enum Color {
    Red,
    Green,
    Blue
}

export enum StringColor {
    Red = "red",
    Green = "green",
    Blue = "blue"
}

export function createColorRed(): Color {
    return Color.Red;
}

export function createStringColorGreen(): StringColor {
    return StringColor.Green;
}

export class EnumContainer {
    public color: Color = Color.Blue;
    public stringColor: StringColor = StringColor.Red;
}

export function createEnumContainer(): EnumContainer {
    return new EnumContainer();
}

// Classes for testing nested ETS object serialization (JSON.stringify(a.b))
export class InnerObj {
    public value: string = 'inner';
    public count: int = 42;
}

export class OuterObj {
    public label: string = 'outer';
    public inner: InnerObj = new InnerObj();
}

export class DeepNestedObj {
    public name: string = 'deep';
    public child: OuterObj = new OuterObj();
}

export function createOuterObj(): OuterObj {
    return new OuterObj();
}

export function createDeepNestedObj(): DeepNestedObj {
    return new DeepNestedObj();
}

// BigInt precision test classes — use default field serialization (path B),
// not JsonSerializable (path A), so that BigInt fields appear as unquoted numeric
// literals in the JSON and get processed by PreprocessBigIntInJson in JSJsonParse.
// Large integer: 12345678901234567890 (> MAX_SAFE_INTEGER 9007199254740991)
export class BigIntPositiveClass {
    public bigValue: BigInt = 12345678901234567890n;
}

export class BigIntNegativeClass {
    public bigNeg: BigInt = -12345678901234567890n;
}

// 9007199254740992 = MAX_SAFE_INTEGER + 1 (unsafe), 9007199254740991 = MAX_SAFE_INTEGER (safe)
export class BigIntSafeBoundaryClass {
    public unsafe: BigInt = 9007199254740992n;
    public safe: BigInt = 9007199254740991n;
}

export class BigIntMixedClass {
    public big: BigInt = 12345678901234567890n;
    public small: int = 42;
    public name: string = "hello";
}

export class BigIntInnerClass {
    public innerBig: BigInt = 12345678901234567890n;
}

export class BigIntNestedClass {
    public outer: BigIntInnerClass = new BigIntInnerClass();
}

export class BigIntMultipleClass {
    public a: BigInt = 12345678901234567890n;
    public b: BigInt = 98765432109876543210n;
}

export class BigIntZeroClass {
    public zero: BigInt = 0n;
}

export class BigIntFloatClass {
    public pi: double = 3.141592653589793;
}

export function createBigIntPositive(): BigIntPositiveClass {
    return new BigIntPositiveClass();
}

export function createBigIntNegative(): BigIntNegativeClass {
    return new BigIntNegativeClass();
}

export function createBigIntSafeBoundary(): BigIntSafeBoundaryClass {
    return new BigIntSafeBoundaryClass();
}

export function createBigIntMixed(): BigIntMixedClass {
    return new BigIntMixedClass();
}

export function createBigIntInner(): BigIntInnerClass {
    return new BigIntInnerClass();
}

export function createBigIntNested(): BigIntNestedClass {
    return new BigIntNestedClass();
}

export function createBigIntMultiple(): BigIntMultipleClass {
    return new BigIntMultipleClass();
}

export function createBigIntZero(): BigIntZeroClass {
    return new BigIntZeroClass();
}

export function createBigIntFloat(): BigIntFloatClass {
    return new BigIntFloatClass();
}


// Unicode char/string fields — all use \uXXXX escapes for CI safety
export class UnicodeNullClass {
    public value: char = c'\u0000';
    public value2: char = c'\u0001';
    public value3: char = c'\u0002';
    public value4: char = c'\u0111';
}

export class UnicodeControlClass {
    public lastCtrl: char = c'\u001f';
    public space: char = c'\u0020';
}

export class UnicodeMultibyteClass {
    public cjk: char = c'\u4e2d';
    public heart: char = c'\u2665';
}

// string type — tests SerializeJSONString path (MUTF-8 encoding)
export class UnicodeStringClass {
    public nullInStr: string = "a\u0000b";
    public mix: string = "hello\u4e2dworld";
}

export function createUnicodeNull(): UnicodeNullClass {
    return new UnicodeNullClass();
}

export function createUnicodeControl(): UnicodeControlClass {
    return new UnicodeControlClass();
}

export function createUnicodeMultibyte(): UnicodeMultibyteClass {
    return new UnicodeMultibyteClass();
}

export function createUnicodeString(): UnicodeStringClass {
    return new UnicodeStringClass();
}
