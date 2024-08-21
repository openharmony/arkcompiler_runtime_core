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

const ONE = 1;
const TEN = 10;
const TWENTY = 20;
const PRECISION = 3;

import { DynObjWrapper, C } from './lib1';

function assertEq<T>(a: T, b: T): void {
	console.log(`assertEq: '${a}' === '${b}'`);
	if (a !== b) {
		throw new Error(`assertEq failed: '${a}' === '${b}'`);
	}
}

export function main(): void {
	testImports();
}

function testImports(): void {
	const o = new C();
	assertEq(o.o1.val, 323);
	assertEq(o.o2.val, 324);
	assertEq(o.v1.toFixed(PRECISION), ONE.toFixed(PRECISION));

	const dynObjWrapper = new DynObjWrapper();
	assertEq(dynObjWrapper.dynObj_.val, TEN);
	assertEq(dynObjWrapper.dynObj_.GetDynVal(), TWENTY);
}
