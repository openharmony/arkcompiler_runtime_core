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
  Bigint division by computed zero throws runtime error.
  If the divisor value of the bigint division operator is 0n,
  then a runtime error is thrown during execution.
---*/

function main(): int {
  let suite = new arktest.ArkTestsuite("BigIntDivisionByComputedZero");

  suite.addTest("Positive bigint / computed 0n throws", () => {
    let zero: bigint = 1n - 1n
    arktest.expectError(() => { let x = 48n / zero; }, 'BigInt: division by zero');
  });

  suite.addTest("Negative bigint / computed 0n throws", () => {
    let zero: bigint = 3n - 3n
    arktest.expectError(() => { let x = -48n / zero; }, 'BigInt: division by zero');
  });

  suite.addTest("Zero bigint / computed 0n throws", () => {
    let zero: bigint = 10n - 10n
    arktest.expectError(() => { let x = 0n / zero; }, 'BigInt: division by zero');
  });

  suite.addTest("Large bigint / computed 0n throws", () => {
    let zero: bigint = 5n - 5n
    arktest.expectError(() => { let x = 999999999999999999n / zero; }, 'BigInt: division by zero');
  });

  return suite.run();
}
