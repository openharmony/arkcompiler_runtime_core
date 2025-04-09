/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
export type AnyAlias<T> = ESObject;
export type UnknownAlias<T> = ESObject;
export type SymbolAlias<T> = ESObject;
export type FunctionAlias<T> = ESObject;
export type ObjectAlias<T> = ESObject;
export type CallSignatureAlias<T, R> = ESObject;
export type ConstructorSignatureAlias<T> = ESObject;
export type IndexSignatureAlias<T> = ESObject;
export type IntersectionAlias<T, U> = ESObject;
export type TypeA = ESObject;
export type TypeB = ESObject;
export type Result = IntersectionAlias<TypeA, TypeB>;
export type Person = ESObject;
export type ConditionalAlias<T, U, V> = ESObject;
export type Result2 = ConditionalAlias<number, string, boolean>;
export type MappedAlias<T> = ESObject;
export type Person2 = ESObject;
export type NewPerson = MappedAlias<Person>;
export type TemplateLiteralAlias<T extends string> = ESObject;
export type Result3 = TemplateLiteralAlias<'A' | 'B'>;
export type PickAlias<T, K extends ESObject> = ESObject;
export type Person3 = ESObject;
export type NameOnly = PickAlias<Person, 'name'>;
export type MappedArray<T, U> = Array<(item: T) => U>;
export type ArrayFilter<T> = ESObject;
export type FunctionTransformer<Input, Output, NewInput, NewOutput> = ESObject;
export type FunctionComposer<Input, Middle, Output> = ESObject;
export type ObjectPropertyMapper<T, U> = ESObject;
export type Fox<T> = ESObject;
declare class MyClass<T> {
    value: T;
    constructor(value: T);
}
