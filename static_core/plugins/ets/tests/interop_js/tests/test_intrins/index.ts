/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

export let nop = function () {};

export let identity = function (v) {
	return v;
};

export let stringifyValue = function (v) {
	return typeof v + ':' + v;
};

export let stringifyArgs = function (...args) {
	return args.toString();
};

export let setProtoConstructor = function (v) {
	return Object.getPrototypeOf(v).constructor;
};

export let applyArgs = function (fn, ...args) {
	return fn(...args);
};

export let throwValue = function (v) {
	throw v;
};

export let log = function (...args) {
	print(`${args.join(' ')}`);
};

export let getProp = function (v, p) {
	return v[p];
};

export let sum = function (a, b) {
	return a + b;
};

export let makeCar = function (v) {
	this.color = v;
};

export let makeTestProxy = function () {
	Object.defineProperty(this, 'foo', {
		get: function () {
			throw Error('get exception');
		},
		set: function () {
			throw Error('set exception');
		},
	});
};
