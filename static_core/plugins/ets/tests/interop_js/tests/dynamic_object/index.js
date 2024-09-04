/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
function regularFunc() {
	return 123;
}
exports.RegularFunc = regularFunc;
exports.Lambda = () => 123;
exports.Foo = {
	foo() {
		return {
			foo() {
				return 123;
			},
		};
	},
};
exports.MultipleCall = () => () => 123;
exports.ArrayOfFoo = [
	{
		foo() {
			return 123;
		},
	},
];
exports.DoubleArrayOfFoo = [
	[
		{
			foo() {
				return 123;
			},
		},
	],
];
exports.ArrayOfLambda = [() => 123];
