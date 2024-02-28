'use strict';
let __extends =
	(this && this.__extends) ||
	(function () {
		let extendStatics = function (d, b) {
			extendStatics =
				Object.setPrototypeOf ||
				({ __proto__: [] } instanceof Array &&
					function (d, b) {
						d.__proto__ = b;
					}) ||
				function (d, b) {
					for (let p in b) {
						if (Object.prototype.hasOwnProperty.call(b, p)) {
							d[p] = b[p];
						}
					}
				};
			return extendStatics(d, b);
		};
		return function (d, b) {
			if (typeof b !== 'function' && b !== null) {
				throw new TypeError('Class extends value ' + String(b) + ' is not a constructor or null');
			}
			extendStatics(d, b);
			function Ctor() {
				this.constructor = d;
			}
			d.prototype = b === null ? Object.create(b) : ((Ctor.prototype = b.prototype), new Ctor());
		};
	})();
exports.__esModule = true;
exports.handler = exports.createInt8Array = exports.createStringRecord = exports.returnCustomArray = exports.CustomArray = exports.getArrey = void 0;
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
function getArray(arr) {
	return arr;
}
exports.getArray = getArray;
let customArray = /** @class */ (function (_super) {
	__extends(customArray, _super);
	function customArray(...args) {
		let items = [];
		for (let _i = 0; _i < args.length; _i++) {
			items[_i] = args[_i];
		}
		return _super.apply(this, items) || this;
	}
	return customArray;
})(Array);
exports.CustomArray = customArray;
function returnCustomArray(array) {
	return array;
}
exports.returnCustomArray = returnCustomArray;
function createStringRecord() {
	let data = {
		key1: 'A',
		key2: 'B',
		key3: 'C',
	};
	return data;
}
exports.createStringRecord = createStringRecord;
function createInt8Array(length) {
	let array = new Int8Array(length);
	for (let i = 0; i < length; i++) {
		array[i] = i;
	}
	return array;
}
exports.createInt8Array = createInt8Array;
exports.handler = {
	get: function (target, property) {
		if (property in target) {
			return target[property];
		} else {
			return undefined;
		}
	},
	set: function (target, property, value) {
		if (typeof value === 'string') {
			target[property] = value;
			return true;
		} else {
			throw new TypeError('The value must be a string');
		}
	},
};
