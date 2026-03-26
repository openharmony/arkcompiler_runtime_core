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
desc: Direct call this() can only be used in explicit constructor call.
assert: The keyword this in a direct call expression this(arguments) can only be used in an explicit constructor call statement.
tags: [compile-only, negative]
expected_out: "Semantic error ESE0201: Call to 'this' must be first statement in constructor"
---*/

class A {
  method(): void {
    this();
  }
}
