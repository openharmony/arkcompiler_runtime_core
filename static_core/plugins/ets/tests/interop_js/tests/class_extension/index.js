'use strict';
/*
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
Object.defineProperty(exports, '__esModule', { value: true });
exports.jsRespectsStaticModifier =
	exports.jsRespectsProtectedModifier =
	exports.extendNativeClass =
	exports.extendUserClass =
	exports.TestNativeClass =
	exports.TestUserClass =
	exports.helperIsJSInstanceOf =
	exports.DEFAULT_NUMERIC_VALUE =
	exports.DEFAULT_STRING_VALUE =
		void 0;
//@ts-ignore -- to avoid @types/node missing
const stream1 = require('stream');
//@ts-ignore -- to avoid @types/node missing
const etsVm = require('lib/module/ets_interop_js_napi');
const ETS_MODULE_NAME = 'interop_class_extension';
const makePrefix = (packageName) => 'L' + packageName.replace('.', '/') + '/';
const getClass = (name, packageName) => etsVm.getClass(makePrefix(packageName ?? ETS_MODULE_NAME) + name + ';');
const getFunction = (name, packageName) => etsVm.getFunction(makePrefix(packageName ?? ETS_MODULE_NAME) + 'ETSGLOBAL;', name);
exports.DEFAULT_STRING_VALUE = 'Panda';
//@ts-ignore -- to avoid @types/node missing
exports.DEFAULT_NUMERIC_VALUE = Math.round(Math.random() * Number.MAX_SAFE_INTEGER);
const helperIsJSInstanceOf = (obj, classObj) => {
	return obj instanceof classObj;
};
exports.helperIsJSInstanceOf = helperIsJSInstanceOf;
class TestUserClass {
	static isMyInstance(obj) {
		return obj instanceof TestUserClass;
	}
	constructor(constructorArg) {
		this.rivateValue = exports.DEFAULT_STRING_VALUE;
		this.protectedValue = exports.DEFAULT_STRING_VALUE;
		this.publicValue = exports.DEFAULT_STRING_VALUE;
		this.readonlyValue = exports.DEFAULT_STRING_VALUE;
		this.constructorSetValue = constructorArg;
	}
}
exports.TestUserClass = TestUserClass;
exports.TestNativeClass = stream1.EventEmitter;
const extendUserClass = () => {
	const TSTestUserClass = getClass('TSTestUserClass');
	class ExtendedEtsUserClass extends TSTestUserClass {}
	const classInstance = new ExtendedEtsUserClass();
	return classInstance instanceof ExtendedEtsUserClass;
};
exports.extendUserClass = extendUserClass;
const extendNativeClass = () => {
	const TSTestNativeClass = getClass('ArrayBuffer', 'escompat');
	class ExtendedEtsNativeClass extends TSTestNativeClass {
		constructor(length) {
			super(length);
		}
	}
	const classInstance = new ExtendedEtsNativeClass(2);
	return classInstance instanceof TSTestNativeClass;
};
exports.extendNativeClass = extendNativeClass;
const jsRespectsProtectedModifier = () => {
	const TSTestUserClass = getClass('TSTestUserClass');
	try {
		class ExtendedEtsUserClass extends TSTestUserClass {
			constructor() {
				super();
				//@ts-ignore -- to ignore compile-time check, as TSTestUserClass is only acquired in runtime
				this.protectedProperty = 'redefinedProtected';
			}
			validProtectedGetter() {
				//@ts-ignore -- to ignore compile-time check, as TSTestUserClass is only acquired in runtime
				return this.protectedProperty;
			}
		}
		const classInstance = new ExtendedEtsUserClass();
		return classInstance.validProtectedGetter() === 'redefinedProtected';
	} catch (e) {
		return false;
	}
};
exports.jsRespectsProtectedModifier = jsRespectsProtectedModifier;
const jsRespectsStaticModifier = () => {
	const TSTestUserClass = getClass('TSTestUserClass');
	class ExtendedEtsUserClass extends TSTestUserClass {}
	//@ts-ignore -- to ignore compile-time check, as TSTestUserClass is only acquired in runtime
	return ExtendedEtsUserClass?.staticProperty === 'static';
};
exports.jsRespectsStaticModifier = jsRespectsStaticModifier;
