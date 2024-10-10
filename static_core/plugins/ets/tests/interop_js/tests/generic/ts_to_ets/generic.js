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

const tsString = 'string';
const tsNumber = 1;

exports.tsString = tsString;
exports.tsNumber = tsNumber;
exports.LiteralClass = class LiteralClass {
	constructor(value) {
		this._value = value;
	}
	set(arg) {
		this._value = arg;
	}
	get() {
		return this._value;
	}
};
exports.UnionClass = class UnionClass {
	constructor(value) {
		this._value = value;
	}
	set(arg) {
		this._value = arg;
	}
	get() {
		return this._value;
	}
};
exports.InterfaceClass = class InterfaceClass {
	constructor(value) {
		this.value = value;
	}
	set(arg) {
		this.value = arg;
	}
	get() {
		return this.value;
	}
};
exports.GAbstract = class GAbstract {
	constructor(value) {
		this._value = value;
	}
	set(arg) {
		this._value = arg;
	}
	get() {
		return this._value;
	}
};
exports.AbstractClass = class AbstractClass extends exports.GAbstract {
	constructor(value) {
		super(value);
	}
};
exports.GClass = class GClass {
	constructor(content) {
		this.content = content;
	}
	get() {
		return this.content;
	}
};
exports.genericFunction = function genericFunction(arg) {
	return arg;
};
exports.tupleDeclaredType = function tupleDeclaredType(items) {
	return items;
};
exports.genericSubsetRef = function genericSubsetRef(items) {
	return items;
};
exports.explicitlyDeclaredType = () => {
	return tsString;
};
exports.literalClass = new exports.LiteralClass(exports.tsString);
exports.unionClass = new exports.UnionClass(exports.tsString);
exports.interfaceClass = new exports.InterfaceClass(exports.tsString);
exports.abstractClass = new exports.AbstractClass(exports.tsString);
