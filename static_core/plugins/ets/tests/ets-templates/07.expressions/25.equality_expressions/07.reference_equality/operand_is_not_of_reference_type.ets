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

{% for eq in reference_eq %}
/*---
desc: A compile-time error occurs if any operand is not of reference type.
tags: [compile-only, negative]
params: {{eq.left}} {{eq.op}} {{eq.right}}
---*/


function main(): void {
  let a: int = 1;
  let b: String = "Hello";
  let c: String = "Hello";
  assertTrue( {{eq.left}} {{eq.op}} {{eq.right}} )
}

{% endfor %}
