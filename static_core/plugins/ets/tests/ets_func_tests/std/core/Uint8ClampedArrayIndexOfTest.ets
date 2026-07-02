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

function getIndexOfTestName(testCase: string): string {
	return 'Uint8ClampedArray.indexOf - ' + testCase;
}

// Regression guard: indexOf/lastIndexOf must NOT narrow the search element to
// the element type before comparison. A negative search element (e.g. -1) must
// not wrap to 255 (modulo 2^8) and falsely match a stored 255.
function testIndexOfNegative(): void {
	const arr = new Uint8ClampedArray([0, 1, 255]);
	arktest.assertEQ(arr.indexOf(-1), -1);   // -1 must not match 255
	arktest.assertEQ(arr.indexOf(-2), -1);
	arktest.assertEQ(arr.indexOf(-255), -1);
	arktest.assertEQ(arr.indexOf(-256), -1);
}

function testLastIndexOfNegative(): void {
	const arr = new Uint8ClampedArray([0, 1, 255, 1]);
	arktest.assertEQ(arr.lastIndexOf(-1), -1);   // lastIndexOf shares the same bug
	arktest.assertEQ(arr.lastIndexOf(-2), -1);
}

function testIndexOfStillFindsPresent(): void {
	const arr = new Uint8ClampedArray([0, 1, 255, 1]);
	arktest.assertEQ(arr.indexOf(0), 0);
	arktest.assertEQ(arr.indexOf(1), 1);
	arktest.assertEQ(arr.indexOf(255), 2);
	arktest.assertEQ(arr.lastIndexOf(1), 3);
	arktest.assertEQ(arr.lastIndexOf(255), 2);
}

function testIndexOfOutOfRangePositive(): void {
	const arr = new Uint8ClampedArray([0, 1, 255]);
	// Out-of-range positives must not narrow/truncate to a stored value.
	arktest.assertEQ(arr.indexOf(256), -1);
	arktest.assertEQ(arr.indexOf(300), -1);
	arktest.assertEQ(arr.lastIndexOf(256), -1);
}

function testIndexOfEmpty(): void {
	const arr = new Uint8ClampedArray(0);
	arktest.assertEQ(arr.indexOf(-1), -1);
	arktest.assertEQ(arr.indexOf(0), -1);
}

function main(): int {
	const suite = new arktest.ArkTestsuite('Uint8ClampedArrayIndexOf');
	suite.addTest(getIndexOfTestName('Negative Search'), testIndexOfNegative);
	suite.addTest(getIndexOfTestName('LastIndexOf Negative'), testLastIndexOfNegative);
	suite.addTest(getIndexOfTestName('Still Finds Present'), testIndexOfStillFindsPresent);
	suite.addTest(getIndexOfTestName('Out Of Range Positive'), testIndexOfOutOfRangePositive);
	suite.addTest(getIndexOfTestName('Empty'), testIndexOfEmpty);
	return suite.run();
}
