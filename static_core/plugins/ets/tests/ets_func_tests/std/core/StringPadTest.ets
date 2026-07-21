/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

function main(): int {
   const suite = new arktest.ArkTestsuite('String API test for SDK');
   suite.addTest('String.padEnd() fills in the correct characters when str is empty', testStringPadEnd);
   suite.addTest('String.padStart() fills in the correct characters when str is empty', testStringPadStart);
   return suite.run()
}

class TestData {
   public static readonly str = 'abc';
   public static readonly padendLength = 5;
   public static readonly padstartLength = 4;
   public static readonly expected = 'abc'

}

function testStringPadEnd() {
   let str = TestData.str;
   let a = str.padEnd(TestData.padendLength, '')
   arktest.assertEQ(TestData.expected, a, `Expected: "${TestData.expected}", Actual: "${a}"`);
}

function testStringPadStart() {
   let str = TestData.str;
   let a = str.padStart(TestData.padstartLength, '')
   arktest.assertEQ(TestData.expected, a, `Expected: "${TestData.expected}", Actual: "${a}"`);
}
