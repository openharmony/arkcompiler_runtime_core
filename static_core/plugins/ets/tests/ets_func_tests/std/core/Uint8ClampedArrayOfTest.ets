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

function getOfTestName(testCase: string): string {
	return 'Uint8ClampedArray.of - ' + testCase;
}

function testOfRoundHalfToEven(): void {
	// ToUint8Clamp round-half-to-even (banker's rounding)
	const ta = Uint8ClampedArray.of(127.5, 126.5, 1.5, 2.5, 0.5, 1.2, 1.8);
	arktest.assertEQ(ta.length, 7);
	arktest.assertEQ(ta.$_get(0).toDouble(), 128); // 127.5 → 128 (127 odd → round up)
	arktest.assertEQ(ta.$_get(1).toDouble(), 126); // 126.5 → 126 (126 even → keep)
	arktest.assertEQ(ta.$_get(2).toDouble(), 2); // 1.5 → 2 (1 odd → round up)
	arktest.assertEQ(ta.$_get(3).toDouble(), 2); // 2.5 → 2 (2 even → keep)
	arktest.assertEQ(ta.$_get(4).toDouble(), 0); // 0.5 → 0 (0 even → keep)
	arktest.assertEQ(ta.$_get(5).toDouble(), 1); // 1.2 → 1 (< 0.5 → down)
	arktest.assertEQ(ta.$_get(6).toDouble(), 2); // 1.8 → 2 (> 0.5 → up)
}

function testOfClampBoundaries(): void {
	const ta = Uint8ClampedArray.of(NaN, -10, 0, 255, 300, Infinity, -Infinity);
	arktest.assertEQ(ta.length, 7);
	arktest.assertEQ(ta.$_get(0).toDouble(), 0); // NaN → 0
	arktest.assertEQ(ta.$_get(1).toDouble(), 0); // -10 → 0
	arktest.assertEQ(ta.$_get(2).toDouble(), 0); // 0 → 0
	arktest.assertEQ(ta.$_get(3).toDouble(), 255); // 255 → 255
	arktest.assertEQ(ta.$_get(4).toDouble(), 255); // 300 → 255
	arktest.assertEQ(ta.$_get(5).toDouble(), 255); // Infinity → 255
	arktest.assertEQ(ta.$_get(6).toDouble(), 0); // -Infinity → 0
}

function testOfBasicInts(): void {
	const ta = Uint8ClampedArray.of(0, 1, 100, 200, 254, 255);
	arktest.assertEQ(ta.length, 6);
	arktest.assertEQ(ta.$_get(0).toDouble(), 0);
	arktest.assertEQ(ta.$_get(1).toDouble(), 1);
	arktest.assertEQ(ta.$_get(2).toDouble(), 100);
	arktest.assertEQ(ta.$_get(3).toDouble(), 200);
	arktest.assertEQ(ta.$_get(4).toDouble(), 254);
	arktest.assertEQ(ta.$_get(5).toDouble(), 255);
}

function testOfEmpty(): void {
	const ta = Uint8ClampedArray.of();
	arktest.assertEQ(ta.length, 0);
}

function main(): int {
	const suite = new arktest.ArkTestsuite('Uint8ClampedArrayOf');
	suite.addTest(getOfTestName('Round Half To Even'), testOfRoundHalfToEven);
	suite.addTest(getOfTestName('Clamp Boundaries'), testOfClampBoundaries);
	suite.addTest(getOfTestName('Basic Ints'), testOfBasicInts);
	suite.addTest(getOfTestName('Empty'), testOfEmpty);
	return suite.run();
}
