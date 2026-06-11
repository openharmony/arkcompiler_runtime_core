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
    Generic library API. `Producer<U>` is a function-type alias; the
    helper `callAndReturn` uses ReturnType<Producer<U>> in its return
    position. When the consumer instantiates U with a concrete type,
    the checker must substitute through both the alias and the
    ReturnType<> wrapper.
tags: [not-a-test, compile-only]
name: lib_factory
arktsconfig: arktsconfig-lib.json
es2panda_options: ['emit-metadata']
---*/

export type Producer<U> = () => U

export function callAndReturn<U>(p: Producer<U>): ReturnType<Producer<U>> {
    return p()
}

export function constantProducer<U>(value: U): Producer<U> {
    return (): U => value
}
