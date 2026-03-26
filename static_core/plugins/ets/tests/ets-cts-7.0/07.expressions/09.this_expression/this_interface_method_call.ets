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
desc: Calling interface methods through this.
assert: The keyword this can call interface methods from within another interface method.
---*/

interface A {
  helper(): int {
    return 11;
  }

  compute(): int {
    let value = this.helper();
    return value * 2;
  }
}

class AImpl implements A {}

function main(): void {
  let a: AImpl = new AImpl();
  arktest.assertEQ(a.helper(), 11);
  arktest.assertEQ(a.compute(), 22);
}
