/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
desc: Declaring method with same name.
assert: It is a compile-time error if a type declaration T has a member method m1 and there exists a method m2 declared in T or a supertype of T such that all of the following are true. m1 and m2 have the same name. m2 is accessible from T . The signature of m1 is not a subsignature of the signature of m2. The signature of m1 or some method m1 overrides (directly or indirectly) has the same erasure as the signature of m2 or some method m2 overrides (directly or indirectly).
tags: [compile-only, negative]
---*/

class C {
  public m(): int {
    return 5;
  }

  public m(): boolean {
    return true;
  }
}
