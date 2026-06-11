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
    Source package defines a class AND exports a type alias 
    built from a utility type over that class. 
    The utility-type alias is serialized and when a
    consumer in another package imports anything from this library,
    parsing of that serialization should succeed
tags: [not-a-test, compile-only]
name: lib_alias
arktsconfig: arktsconfig-lib.json
es2panda_options: ['emit-declaration']
---*/

export class Settings {
    theme: string = "dark"
    fontSize: int = 12
    autoSave: boolean = true
}

export type PartialSettings = Partial<Settings>
export type ReadonlySettings = Readonly<Settings>
