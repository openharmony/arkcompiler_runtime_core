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
desc: Chaining operator, method chaining call.
tags: [compile-only, negative]
---*/

class Builder {
  private value = 0;
  add(n: number) {
    this.value += n;
    return this;
  }
  getResult() { return this.value; }
}

function main(): void {
  const builder: Builder | null = null;
  assertEQ(builder?.add(5), getResult());
}