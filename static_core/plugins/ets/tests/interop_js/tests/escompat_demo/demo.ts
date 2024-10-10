/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { FooClass, FooFunction, BarFunction } from './foo';

function test(): void {
	let arr = new Array<FooClass>();
	arr.push(new FooClass('zero'));
	arr.push(new FooClass('one'));
	arr.push(new FooClass('two'));

	console.log('test: ' + FooFunction(arr));

	let arr2 = BarFunction();
	console.log('test: check instanceof Array: ' + (arr2 instanceof Array));

	console.log('test: ' + arr2.at(0).name);
	console.log('test: ' + arr2.at(1).name);
	console.log('test: ' + arr2.at(2).name);
}
test();
