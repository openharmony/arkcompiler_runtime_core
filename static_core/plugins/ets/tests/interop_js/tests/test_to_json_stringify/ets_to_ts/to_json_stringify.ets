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

export class DerivedClass extends BaseClass {}

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
