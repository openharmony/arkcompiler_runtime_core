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
    Declaration file resolved via relative import and extension fallback.
    Triggers SOURCE_DECL through EndsWith(ResolvedSource(), ".d.ets") branch
    in ImportPathManager::LookupSourceFile (line 1270).
tags: [not-a-test, compile-only]
name: decl_types
---*/

'use static'

export declare class Config {
    public name: string;
    public value: int;
    public constructor(name: string, value: int);
}
