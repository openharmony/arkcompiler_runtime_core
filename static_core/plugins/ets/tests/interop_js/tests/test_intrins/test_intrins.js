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
'use strict';

let etsVm = gtest.etsVm;
exports.etsVm = etsVm;

exports.nop = function () {};
exports.identity = function (v) {
	return v;
};
exports.stringifyValue = function (v) {
	return typeof v + ':' + v;
};
exports.stringifyArgs = function (...args) {
	return args.toString();
};
exports.setProtoConstructor = function (v) {
	return Object.getPrototypeOf(v).constructor;
};
exports.applyArgs = function (fn, ...args) {
	return fn(...args);
};
exports.throwValue = function (v) {
	throw v;
};
exports.log = function (...args) {
	console.log(...args);
};
exports.getProp = function (v, p) {
	return v[p];
};

exports.sum = function (a, b) {
	return a + b;
};
exports.makeCar = function (v) {
	this.color = v;
};

exports.makeTestProxy = function () {
	Object.defineProperty(this, 'foo', {
		get: function () {
			throw Error('get exception');
		},
		set: function () {
			throw Error('set exception');
		},
	});
};
