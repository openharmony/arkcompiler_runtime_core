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
desc: Bigint greater-than-or-equal operator - returns boolean
---*/

function main(): int {
  let suite = new arktest.ArkTestsuite("BigIntGreaterThanOrEqual");

  suite.addTest("Equal values", () => {
    arktest.assertEQ(0n >= 0n, true);
    arktest.assertEQ(5n >= 5n, true);
    arktest.assertEQ(-10n >= -10n, true);
  });

  suite.addTest("Positive >= Positive", () => {
    arktest.assertEQ(2n >= 1n, true);
    arktest.assertEQ(1n >= 2n, false);
    arktest.assertEQ(100n >= 100n, true);
    arktest.assertEQ(200n >= 100n, true);
  });

  suite.addTest("Negative >= Negative", () => {
    arktest.assertEQ(-3n >= -5n, true);
    arktest.assertEQ(-5n >= -3n, false);
    arktest.assertEQ(-100n >= -100n, true);
  });

  suite.addTest("Positive >= Negative", () => {
    arktest.assertEQ(1n >= -1n, true);
    arktest.assertEQ(0n >= -1n, true);
    arktest.assertEQ(10n >= -5n, true);
  });

  suite.addTest("Negative >= Positive", () => {
    arktest.assertEQ(-1n >= 1n, false);
    arktest.assertEQ(-1n >= 0n, false);
    arktest.assertEQ(-10n >= 5n, false);
  });

  suite.addTest("Zero comparisons", () => {
    arktest.assertEQ(0n >= 0n, true);
    arktest.assertEQ(1n >= 0n, true);
    arktest.assertEQ(0n >= 1n, false);
    arktest.assertEQ(0n >= -1n, true);
  });

  suite.addTest("Large numbers", () => {
    arktest.assertEQ(9223372036854775808n >= 9223372036854775807n, true);
    arktest.assertEQ(9223372036854775807n >= 9223372036854775807n, true);
  });

  return suite.run();
}
