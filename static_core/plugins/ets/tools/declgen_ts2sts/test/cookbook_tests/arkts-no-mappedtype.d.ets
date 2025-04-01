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
export type Class1<T> = ESObject;
export type Nullable<T> = ESObject;
export type Person = ESObject;
export type ReadonlyPerson = ESObject;
export type ArrayElementMap<T extends ESObject[], U> = ESObject;
export type NumberArray = [number, number, number];
export type StringArray = ArrayElementMap<NumberArray, string>;
export type Book = ESObject;
export type SelectedKeys = 'title' | 'author';
export type PartialBook = ESObject;
export type a = Person;
export declare class Class2<T> {
    a: Person;
}
