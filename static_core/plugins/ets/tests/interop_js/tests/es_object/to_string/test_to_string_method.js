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

exports.Identity = function (x) {
	return x;
};
exports.Curry = function (x) {
	return {
		toString() {
			return x;
		},
	};
};
exports.Throwing = function () {
	return {
		toString() {
			throw 123;
		},
	};
};
exports.CurryToString = function (x) {
	return {
		toString() {
			return x.toString();
		},
	};
};
exports.IntValue = function () {
	return 42;
};
exports.NullVal = function () {
	return null;
};
exports.NanVal = function () {
	return NaN;
};
exports.UndefinedVal = function () {
	return undefined;
};
exports.Optional = function (x) {
	return x ? null : 'optionalVal';
};
exports.StringifyValue = function (v) {
	return typeof v + ':' + v;
};
