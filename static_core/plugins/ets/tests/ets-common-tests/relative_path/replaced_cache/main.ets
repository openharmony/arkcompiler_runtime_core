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
  tags: [compile-only]
  files: [shared.ets]
  arktsconfig: arktsconfig.json
  es2panda_options: [simultaneous]
---*/

import { Shared } from 'cacheDedup/shared';
import { CacheLeftImporter } from 'cacheDedup/left';
import { CacheRightImporter } from 'cacheDedup/right';

// Simultaneous compilation registers shared.ets as an implementation before resolving imports.
// The two cached importers then resolve './shared' to the same replaced cache declaration.
function sumCachedValues(left: CacheLeftImporter, right: CacheRightImporter): int {
    let leftValue: Shared = left.shared;
    let rightValue: Shared = right.shared;
    return leftValue.value + rightValue.value;
}

function main(): void {}
