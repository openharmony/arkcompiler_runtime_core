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
  arktsconfig: arktsconfig_MissingSourceTarget.json
---*/

// An unavailable original target falls back to the existing logical key.
import { Foo } from '@test/pkg/src/Nested';
import { Foo as DirectFoo } from '@test/pkg/Foo';

function sameTarget(value: Foo): DirectFoo {
    return value;
}

function readDeclgenMarker(value: Foo): int {
    return value.declgenMarker;
}

function main(): int {
    return 0;
}
