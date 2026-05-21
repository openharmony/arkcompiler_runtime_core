/*
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

/*---
desc: Package @panda/utils - utility functions (sequential, depends on @panda/types)
tags: [not-a-test, compile-only]
files: [lib_types.ets]
name: lib_utils
arktsconfig: arktsconfig-utils.json
es2panda_options: ['emit-declaration']
---*/

import { StringOrNull, Callback, Mapper, IntList, IntNode, Score, StringOrNum, Issue, OptIssue, StringProducer, Container } from '@panda/types/lib_types'

// --- Optional parameters ---
export function greet(name: string, prefix: string = "Hello"): string {
    return prefix + " " + name
}

// --- Closure / lambda ---
export function makeAdder(base: int): (x: int) => int {
    return (x: int): int => {
        return base + x
    }
}

// --- Nullish return ---
export function maybeString(flag: boolean): StringOrNull {
    if (flag) return "yes"
    return null
}

// --- Callback usage (generic type alias from types) ---
export function applyCallback<T>(val: T, cb: Callback<T>): string {
    return cb(val)
}

// --- Lambda factory using Mapper type alias ---
export function createMultiplier(factor: int): Mapper<int, int> {
    return (input: int): int => {
        return input * factor
    }
}

// --- Recursive type alias (linked list traversal) ---
export function listSum(list: IntList): int {
    if (list == null) return 0
    return list.head + listSum(list.tail)
}

export function makeList(n: int): IntList {
    if (n <= 0) return null
    return new IntNode(n, makeList(n - 1))
}

// --- Rest parameter ---
export function sumAll(...nums: int[]): int {
    let total: int = 0
    for (let n of nums) {
        total += n
    }
    return total
}

// --- Numeric widening (int → number) ---
export function quarterOf(n: number): string {
    return (n / 4.0).toString()
}

// --- Type alias interchangeability (Score ↔ int) ---
export function addScores(a: Score, b: Score): Score {
    return a + b
}

// --- Union type (string | number) ---
export function describeValue(val: StringOrNum): string {
    return "val:" + val.toString()
}

// --- Boxing (primitive → Object) ---
export function boxToString(obj: Object): string {
    return obj.toString()
}

// --- Void return type ---
export function pushItem(arr: string[], item: string): void {
    arr.push(item)
}

// --- Never return type ---
export function failWith(msg: string): never {
    throw new Error(msg)
}

// --- Partial<T> utility type ---
export function formatPartial(issue: Partial<Issue>): string {
    let t = issue.title ?? "untitled"
    let d = issue.description ?? "none"
    return t + ":" + d
}

// --- Required<T> utility type ---
export function formatRequired(issue: Required<OptIssue>): string {
    return issue.title + ":" + issue.description
}

// --- Readonly<T> utility type ---
export function readonlyTitle(issue: Readonly<Issue>): string {
    return issue.title
}

// --- Record<K, V> utility type ---
export function getFromRecord(rec: Record<string, string>, key: string): string {
    return rec[key] ?? ""
}

// --- NonNullable<T> utility type ---
export function ensureNonNull(val: StringOrNull): NonNullable<StringOrNull> {
    if (val == null) return "default"
    return val
}

// --- ReturnType<T> utility type ---
export function callProducer(fn: StringProducer): ReturnType<StringProducer> {
    return fn()
}

// --- Awaited<T> utility type ---
export function awaitedLabel(val: Awaited<Promise<string>>): string {
    return "awaited:" + val
}

// --- Array<T> as cross-package parameter ---
export function arrayConcat(arr: Array<string>): string {
    let result = ""
    for (let s of arr) {
        result += s
    }
    return result
}

// --- Tuple as cross-package return type ---
export function makePair(name: string, value: int): [string, int] {
    return [name, value]
}

// --- BigInt across packages ---
export function bigIntAdd(a: bigint, b: bigint): string {
    return (a + b).toString()
}

// --- Object as field type across packages ---
export function containerLabel(c: Container): string {
    return c.label + "=" + c.content.toString()
}

// --- Shadowing (local variable shadows module-level function 'greet') ---
export function testShadowing(): string {
    let greet = "local-greet"
    return greet
}

// --- Promise factory ---
export function wrapInPromise(val: string): Promise<string> {
    return new Promise<string>((resolve: (v: string) => void): void => {
        resolve(val)
    })
}
