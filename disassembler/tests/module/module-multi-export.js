/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*---
flags: [module]
---*/


export let a = 1
export let b = 2
export let c = 3
export let d = 5
export let e = 6
export let f = 7
export let g = 8

let x = 4
export { x } from './module-multi-import.js'
export * from "./module-multi-import.js"
export { a, b, c }
export { d as dd, e as ee }
export class ClassB {
    fun(d, e){ return d + e }
}
export function FunctionC() {
    let h = 9
}