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
desc: >-
  Bigint with computed negative exponent throws runtime error.
  If the second operand of type bigint is negative,
  then a runtime error is thrown during execution.
---*/

function main(): int {
  let suite = new arktest.ArkTestsuite("BigIntNegativeExponent");

  suite.addTest("Positive base computed negative exponent", () => {
    let neg: bigint = 0n - 1n
    arktest.expectError(() => { let x = 2n ** neg; }, 'Exponent must be non-negative');
  });

  suite.addTest("Negative base computed negative exponent", () => {
    let neg: bigint = 0n - 1n
    arktest.expectError(() => { let x = (-2n) ** neg; }, 'Exponent must be non-negative');
  });

  suite.addTest("Large base computed negative exponent", () => {
    let neg: bigint = 0n - 5n
    arktest.expectError(() => { let x = 100n ** neg; }, 'Exponent must be non-negative');
  });

  suite.addTest("Zero base computed negative exponent", () => {
    let neg: bigint = 0n - 1n
    arktest.expectError(() => { let x = 0n ** neg; }, 'Exponent must be non-negative');
  });

  return suite.run();
}
