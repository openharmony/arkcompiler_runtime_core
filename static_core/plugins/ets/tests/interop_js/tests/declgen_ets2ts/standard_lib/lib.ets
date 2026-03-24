/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

export let s: Set<number> = new Set<number>()

export const weakMap: WeakMap<object, string> = new WeakMap<object, string>();

export let w: WeakSet<object> = new WeakSet<object>()

export let map: Map<string, number> = new Map<string, number>()

export let f32: Float32Array = new Float32Array()

export let f64: Float64Array = new Float64Array()

export let i8: Int8Array = new Int8Array()

export let i16: Int16Array = new Int16Array()

export let i32: Int32Array = new Int32Array()

export let ui8: Uint8Array = new Uint8Array()

export let ui16: Uint16Array = new Uint16Array()

export let ui32: Uint32Array = new Uint32Array()

export let ui8c: Uint8ClampedArray = new Uint8ClampedArray()

const buffer = new ArrayBuffer(16);
export const dataView: DataView = new DataView(buffer);

export let b64: BigInt64Array = new BigInt64Array(2)

export let ub64: BigUint64Array = new BigUint64Array(2)

const regExp: RegExp = new RegExp('abc');
export const matchResult: RegExpExecArray | null = regExp.exec('abcdef');

export const regExp2: RegExp = new RegExp('abc');

export let foo : (...nums: number[]) => void = (...nums: number[]) : void => {}

export type StringDictionary = Record<string, string>

export type Matrix = number[][]
export type Handler = (s: string, no: number) => string
export type Predicate<T> = (x: T) => Boolean
export type NullableNumber = Number | null
export type Dictionary = Map<string, string>