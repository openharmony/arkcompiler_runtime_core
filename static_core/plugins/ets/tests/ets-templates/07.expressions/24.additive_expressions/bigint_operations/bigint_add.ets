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
desc: Bigint addition - no implicit conversion, result is bigint
---*/

function main(): int {
  let suite = new arktest.ArkTestsuite("BigIntAddition");

  suite.addTest("Zero addition", () => {
    arktest.assertEQ(0n + 0n, 0n);
    arktest.assertEQ(0n + 5n, 5n);
    arktest.assertEQ(5n + 0n, 5n);
  });

  suite.addTest("Positive + Positive", () => {
    arktest.assertEQ(2n + 3n, 5n);
    arktest.assertEQ(10n + 20n, 30n);
  });

  suite.addTest("Negative + Negative", () => {
    arktest.assertEQ(-2n + -3n, -5n);
  });

  suite.addTest("Positive + Negative", () => {
    arktest.assertEQ(10n + -3n, 7n);
    arktest.assertEQ(5n + -10n, -5n);
  });

  suite.addTest("Negative + Positive", () => {
    arktest.assertEQ(-10n + 3n, -7n);
    arktest.assertEQ(-5n + 10n, 5n);
  });

  suite.addTest("Large numbers", () => {
    arktest.assertEQ(1000000n + 1000000n, 2000000n);
    arktest.assertEQ(9223372036854775807n + 1n, 9223372036854775808n);
  });

  suite.addTest("Very large bigint", () => {
    let a = 123456789012345678901234567890n;
    let b = 987654321098765432109876543210n;
    arktest.assertEQ(a + b, 1111111110111111111011111111100n);
  });

  return suite.run();
}
