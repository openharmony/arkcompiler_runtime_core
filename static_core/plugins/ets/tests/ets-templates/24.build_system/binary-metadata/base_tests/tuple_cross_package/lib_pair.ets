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
desc: >-
    Library exports tuple-typed function signatures and a type alias.
tags: [not-a-test, compile-only]
name: lib_pair
arktsconfig: arktsconfig-lib.json
es2panda_options: ['emit-metadata']
---*/

export type IntStr = [int, string]

export function makePair(n: int, s: string): IntStr {
    return [n, s]
}

export function describe(p: IntStr): string {
    return "(" + p[0] + "," + p[1] + ")"
}

export function takeMixed(t: [int, string, boolean]): string {
    return t[0] + ":" + t[1] + ":" + t[2]
}
