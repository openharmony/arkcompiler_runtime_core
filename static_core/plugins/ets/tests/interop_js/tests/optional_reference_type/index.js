'use strict';
/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
exports.__esModule = true;
exports.fnWithAnyParamObject =
	exports.fnWithUserInterface =
	exports.fnWithUserClass =
	exports.TestUserClass =
	exports.fnWithUnionParam =
	exports.fnWithSubsetPartial =
	exports.fnWithSubsetOmit =
	exports.fnWithSubsetPick =
	exports.fnWithExtraSetParam =
	exports.fnWithLiteralParam =
	exports.fnWithAnyParams =
		void 0;

function fnWithAnyParams(arr) {
	return arr ? arr[0] : 'Argument not found';
}

exports.fnWithAnyParams = fnWithAnyParams;

function fnWithLiteralParam(arr) {
	return arr ? arr[0] : 'Argument not found';
}

exports.fnWithLiteralParam = fnWithLiteralParam;

function fnWithExtraSetParam(arr) {
	return arr ? arr[0] : 'Argument not found';
}

exports.fnWithExtraSetParam = fnWithExtraSetParam;

function fnWithSubsetPick(obj) {
	return obj.address ? obj.address.city : 'Address not found';
}

exports.fnWithSubsetPick = fnWithSubsetPick;

function fnWithSubsetOmit(obj) {
	return obj.address ? obj.address.city : 'Address not found';
}

exports.fnWithSubsetOmit = fnWithSubsetOmit;

function fnWithSubsetPartial(obj) {
	return obj.address ? obj.address.city : 'Address not found';
}

exports.fnWithSubsetPartial = fnWithSubsetPartial;

function fnWithUnionParam(obj) {
	if (obj) {
		if (Array.isArray(obj)) {
			return 'This is an array';
		}
		return 'This is an object';
	}
	return 'Argument not found';
}

exports.fnWithUnionParam = fnWithUnionParam;

let TestUserClass = /** @class */ (function () {
	function testUserClass(id, name) {
		this.id = id;
		this.name = name;
	}
	return testUserClass;
})();

exports.TestUserClass = TestUserClass;

function fnWithUserClass(obj) {
	return obj ? obj.name : 'Class was not passed';
}

exports.fnWithUserClass = fnWithUserClass;

function fnWithUserInterface(obj) {
	return obj ? obj.name : 'Object was not passed';
}

exports.fnWithUserInterface = fnWithUserInterface;

function fnWithAnyParamObject(obj) {
	return obj.arr ? obj.arr[0] : obj.id;
}

exports.fnWithAnyParamObject = fnWithAnyParamObject;
