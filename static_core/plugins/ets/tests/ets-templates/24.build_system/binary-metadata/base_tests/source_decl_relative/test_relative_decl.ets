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
    Relative import resolving to .d.ets file via extension fallback.
    import './decl_types' resolves to decl_types.d.ets (no decl_types.ets exists).
    Triggers SOURCE_DECL through EndsWith(ResolvedSource(), ".d.ets") branch
    in ImportPathManager::LookupSourceFile (importPathManager.cpp:1270).
tags: [compile-only]
name: test_relative_decl
arktsconfig: arktsconfig.json
---*/

import { Config } from './decl_types'

let c: Config = new Config("test", 42)
let name: string = c.name
