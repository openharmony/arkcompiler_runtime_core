/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

{% for statement in statements %}
/*---
desc: All statements in the module level must be enclosed in curly braces to form a block.
tags: [compile-only]
---*/

let x: int = 0;

function check(): boolean {
  return true;
}

function pass(): void {
  assertTrue( true )
}

{
  {{statement.block}}
}

{% endfor %}
