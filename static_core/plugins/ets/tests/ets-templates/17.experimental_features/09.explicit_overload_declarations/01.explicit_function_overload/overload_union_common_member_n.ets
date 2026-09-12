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
desc: Overload union common member is rejected at compile time.
tags: [compile-only, negative]
expected_out:
  compiler:
    - >-
        Semantic error ESE0244: Member type must be the same for all union objects.
    - >-
        Semantic error ESE0391: Overload declaration cannot be called by union.
---*/

'use static'

class OverloadFail017C {
  readInt(p: int): int {
    return p
  }

  readString(p: string): string {
    return p
  }

  overload read { readInt, readString }
}

class OverloadFail017D {
  read(p: int): int {
    return p
  }
}

function useUnion(v: OverloadFail017C | OverloadFail017D): int {
  return v.read(1)
}
