/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

export const CONSTANT: number = 228;

export class Bibka {
    public static readonly NOT_GLOBAL: number = 1337;
}

export type UT = null|undefined|never
export type UTA = UT[]

export type a1 = "a" | "b" | "c"
export type b1 = Record<string, number |string | boolean | [] | Record<string, number |string | boolean | []>>
export type c1 = Record<number, string> | Record<string, number>
export class B<T> {}
export type A<K,T> = string | B<A<K,T>>
export type Callback<T> = (data: T) => void;
export type ErrorCallback<T extends Error = BusinessError> = (err: T)=> void;
export type AsyncCallback<T, E = void> = (err: BusinessError<E>, data: T)=> void;
export declare class BusinessError<T = void> extends Error {}