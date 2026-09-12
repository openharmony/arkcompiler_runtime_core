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
desc: Callable factory with parameter is rejected at compile time.
tags: [compile-only, negative]
expected_out:
  compiler:
    - >-
        Semantic error ESE37484: Invalid type call expression as there is no static method
        $_instantiate with the first parameter of the function type defined in class CallableFail012
    - >-
        Semantic error ESE0002: Type 'CallableFail012' has no call signatures.
---*/

'use static'

class CallableFail012 {
  static $_instantiate(factory: (value: int) => CallableFail012): CallableFail012 {
    return new CallableFail012()
  }
}

function main(): void {
  CallableFail012()
}
