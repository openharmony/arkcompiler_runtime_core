/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: Verify that a class can distinguish the same - named constants between an interface constant and a class nullish - coalescing property by nullish - coalescing access.
---*/

interface NullishCoalescingConflictInterface {
  readonly CONST: number;
}

class NullishCoalescingConflictClass implements NullishCoalescingConflictInterface {
  readonly CONST: number = 58;
  readonly nullable?: number;
  readonly nullishCONST = this.nullable ?? 59;
}

function main(): void {
  const instance = new NullishCoalescingConflictClass();
  assertEQ(instance.CONST, 58);
  assertEQ(instance.nullishCONST, 59);
}