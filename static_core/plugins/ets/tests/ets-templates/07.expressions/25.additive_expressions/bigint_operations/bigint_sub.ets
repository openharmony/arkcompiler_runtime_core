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
desc: Bigint subtraction - no implicit conversion, result is bigint
---*/

function main(): int {
  let suite = new arktest.ArkTestsuite("BigIntSubtraction");

  suite.addTest("Zero subtraction", () => {
    arktest.assertEQ(0n - 0n, 0n);
    arktest.assertEQ(5n - 0n, 5n);
    arktest.assertEQ(0n - 5n, -5n);
  });

  suite.addTest("Positive - Positive", () => {
    arktest.assertEQ(5n - 3n, 2n);
    arktest.assertEQ(10n - 20n, -10n);
  });

  suite.addTest("Negative - Negative", () => {
    arktest.assertEQ(-5n - -3n, -2n);
  });

  suite.addTest("Positive - Negative", () => {
    arktest.assertEQ(10n - -3n, 13n);
  });

  suite.addTest("Negative - Positive", () => {
    arktest.assertEQ(-10n - 3n, -13n);
  });

  suite.addTest("Self subtraction", () => {
    arktest.assertEQ(39n - 39n, 0n);
  });

  suite.addTest("Large numbers", () => {
    arktest.assertEQ(9223372036854775808n - 1n, 9223372036854775807n);
  });

  return suite.run();
}
