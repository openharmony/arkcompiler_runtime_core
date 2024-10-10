/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

const ZERO = 0;
const ONE = 1;
const TWO = 2;
const ONE_HUNDRED = 100;
const NINETY_NINE = 99;

// NOTE (ivagin) #12808 enable when supported by declgen
const FIX_12808 = false;

import * as imported from './lib';

const { Color, Letter } = imported;

function assertEq<T>(a: T, b: T): void {
	console.log(`assertEq: '${a}' === '${b}'`);
	if (a !== b) {
		throw new Error(`assertEq failed: '${a}' === '${b}'`);
	}
}

export function main(): void {
	testClasses();
}

function testClasses(): void {
	assertEq(Letter.A, ZERO);
	assertEq(Letter.B, ONE);
	assertEq(Letter.C, TWO);

	assertEq(Color.Red, ZERO);
	assertEq(Color.Green, -ONE_HUNDRED);
	assertEq(Color.Blue, -NINETY_NINE);
	assertEq(Color.White, ZERO);
	if (FIX_12808) {
		const workaroundImport = { Ivan: 'John', Li: 'Yuan', ...imported };
		// @ts-ignore -- remove when #12808 is fixed
		const Name = FIX_12808 ? workaroundImport.Name : { Ivan: 'John', Li: 'Yuan' };
		assertEq(Name.Ivan, 'Ivan');
		assertEq(Name.Li, 'Li');
	}
}
