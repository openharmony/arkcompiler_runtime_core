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
desc: Package @panda/app - extension module (simultaneous with app_main)
tags: [not-a-test]
name: app_ext
---*/

import { GenBox, Score, Config as Cfg, makeConfig as createConfig, Priority, StringOrNull, StringOrNum, StringProducer, Container } from '@panda/types/lib_types'
import { NumBox } from '@panda/types/lib_impls'
import { maybeString, applyCallback, createMultiplier, wrapInPromise, sumAll, listSum, makeList, quarterOf, addScores, describeValue, boxToString, pushItem, failWith, formatPartial, formatRequired, readonlyTitle, getFromRecord, ensureNonNull, callProducer, awaitedLabel, arrayConcat, makePair, bigIntAdd, containerLabel, testShadowing } from '@panda/utils/lib_utils'

// --- Generic extension (NumBox extends GenBox<number>) ---
export function testNumBox(): string {
    let nb = new NumBox(21)
    return nb.doubled().toString()
}

// --- Nullish operator with cross-package function ---
export function testNullish(): string {
    let a = maybeString(true) ?? "fallback"
    let b = maybeString(false) ?? "fallback"
    return a + "," + b
}

// --- Generic callback with type alias ---
export function testCallback(): string {
    return applyCallback<int>(42, (v: int): string => { return "val=" + v.toString() })
}

// --- Getter/setter on generic GenBox ---
export function testGenBoxSetter(): string {
    let b = new GenBox<string>("initial")
    b.value = "updated"
    return b.value
}

// --- Lambda with Mapper type alias from cross-package ---
export function testMapper(): string {
    let mul3 = createMultiplier(3)
    return mul3(7).toString()
}

// --- Recursive type alias from cross-package ---
export function testRecursiveType(): string {
    return listSum(makeList(5)).toString()
}

// --- Ambient type alias from cross-package ---
export function testAmbientType(): string {
    let s: Score = 99
    let doubled: Score = s * 2
    return doubled.toString()
}

// --- Rest parameter from cross-package ---
export function testRestParam(): string {
    return sumAll(1, 2, 3, 4).toString()
}

// --- Named reference (import with alias) from cross-package ---
export function testNamedRef(): string {
    let c: Cfg = createConfig(7)
    return c.id.toString() + ":" + c.label
}

// --- Numeric widening (int → number) across packages ---
export function testWidening(): string {
    return quarterOf(7)
}

// --- Enum to int conversion across packages ---
export function testEnumToInt(): string {
    let val: int = Priority.Low + 5
    return val.toString()
}

// --- Nullable widening (string → string|null) across packages ---
export function testNullableWidening(): string {
    let s: StringOrNull = maybeString(true)
    return s ?? "default"
}

// --- Type alias interchangeability (int → Score) across packages ---
export function testScoreAlias(): string {
    let x: int = 10
    let y: int = 20
    let result: Score = addScores(x, y)
    return result.toString()
}

// --- Union type (string | number) across packages ---
export function testUnion(): string {
    let a: StringOrNum = "abc"
    let b: StringOrNum = 42
    return describeValue(a) + "," + describeValue(b)
}

// --- Boxing (primitive → Object) across packages ---
export function testBoxing(): string {
    let a = boxToString(42)
    let b = boxToString(true)
    return a + "," + b
}

// --- Void return type across packages ---
export function testVoid(): string {
    let arr: string[] = ["a"]
    pushItem(arr, "b")
    return arr.length.toString()
}

// --- Never return type across packages ---
export function testNever(): string {
    try {
        failWith("boom")
    } catch (e) {
        return "caught"
    }
    return "unreachable"
}

// --- Partial<T> utility type across packages ---
export function testPartial(): string {
    return formatPartial({title: "bug"})
}

// --- Required<T> utility type across packages ---
export function testRequired(): string {
    return formatRequired({title: "feat", description: "new"})
}

// --- Readonly<T> utility type across packages ---
export function testReadonly(): string {
    return readonlyTitle({title: "doc", description: "readme"})
}

// --- Record<K, V> utility type across packages ---
export function testRecord(): string {
    return getFromRecord({'name': "alice", 'role': "admin"}, "name")
}

// --- NonNullable<T> utility type across packages ---
export function testNonNullable(): string {
    let a = ensureNonNull("hi")
    let b = ensureNonNull(null)
    return a + "," + b
}

// --- ReturnType<T> utility type across packages ---
export function testReturnType(): string {
    let fn: StringProducer = (): string => { return "produced" }
    return callProducer(fn)
}

// --- Awaited<T> utility type across packages ---
export function testAwaited(): string {
    return awaitedLabel("resolved")
}

// --- Array<T> across packages ---
export function testArray(): string {
    return arrayConcat(["a", "b", "c"])
}

// --- Tuple across packages ---
export function testTuple(): string {
    let pair = makePair("score", 42)
    return pair[0] + ":" + pair[1].toString()
}

// --- BigInt across packages ---
export function testBigInt(): string {
    return bigIntAdd(100n, 200n)
}

// --- Object as field type across packages ---
export function testObjectField(): string {
    return containerLabel(new Container("val", 99))
}

// --- Shadowing (local variable shadows module-level function 'greet' ---
export function testShadowingVariable(): string {
    return testShadowing()
}

// --- Promise factory from cross-package ---
export async function testPromise(): Promise<string> {
    let result = await wrapInPromise("resolved-ok")
    return result
}
