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

const tsTestString = 'ts_test_string';
exports.UnionSetter = class UnionSetter {
	set value(arg) {
		this._value = arg;
	}
	get value() {
		return this._value;
	}
};
exports.InterfaceSetter = class InterfaceSetter {
	set value(arg) {
		this._value = arg;
	}
	get value() {
		return this._value;
	}
};
exports.BaseClass = class BaseClass {
	set value(arg) {
		this._value = arg;
	}
	get value() {
		return this._value;
	}
};
exports.SubsetRefSet = class SubsetByRef extends exports.BaseClass {};
exports.tsTestString = tsTestString;
class ValueSetter {
	constructor() {
		this._value = tsTestString;
	}
}
exports.SubsetValueSet = class SubsetValueSet extends ValueSetter {
	constructor() {
		super();
	}
	set value(arg) {
		this._value = arg;
	}
	get value() {
		return this._value;
	}
};
exports.SetterAnyType = class SetterAnyType {
	set value(arg) {
		this._value = arg;
	}
	get value() {
		return this._value;
	}
};
exports.TupleSet = class TupleSet {
	set value(arg) {
		this._value = arg;
	}
	get value() {
		return this._value;
	}
};
class AbstractSetter {}
exports.AbstractClass = class AbstractClass extends AbstractSetter {
	set value(arg) {
		this._value = arg;
	}
	get value() {
		return this._value;
	}
};
exports.AbstractClassObject = new exports.AbstractClass();
exports.UnionSetterObject = new exports.UnionSetter();
exports.InterfaceSetterObject = new exports.InterfaceSetter();
exports.TupleSetObject = new exports.TupleSet();
exports.SetterAnyTypeObject = new exports.SetterAnyType();
exports.BaseClassObject = new exports.BaseClass();
exports.SubsetRefSetObject = new exports.SubsetRefSet();
exports.SubsetValueSetObject = new exports.SubsetValueSet();
