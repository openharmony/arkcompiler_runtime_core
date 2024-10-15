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
const tsInt = 1;
exports.tsInt = tsInt;
exports.MainClass = class MainClass {
	constructor(value) {
		this._value = value;
	}
};
exports.IIFEClass = (function () {
	return class {
		constructor(value) {
			this._value = value;
		}
	};
})();
exports.AnonymousClass = class {
	constructor(value) {
		this._value = value;
	}
};
exports.ParentClass = class ParentClass {
	constructor(otherClass) {
		this._otherClass = new otherClass(tsInt);
	}
};
exports.createClassWithArgFromTs = function createClassWithArgFromTs(arg) {
	return new exports.ParentClass(arg);
};
exports.createMainClassFromTs = function createMainClassFromTs() {
	return new exports.ParentClass(exports.MainClass);
};
exports.mainClassInstance = new exports.ParentClass(exports.MainClass);
exports.createAnonymousClassFromTs = function createAnonymousClassFromTs() {
	return new exports.ParentClass(exports.AnonymousClass);
};
exports.anonymousClassInstance = new exports.ParentClass(exports.AnonymousClass);
exports.createIIFEClassFromTs = function createIIFEClassFromTs() {
	return new exports.ParentClass(exports.IIFEClass);
};
exports.iifeClassInstance = new exports.ParentClass(exports.IIFEClass);
exports.ChildClass = class ChildClass extends exports.ParentClass {
	constructor(otherClass) {
		super(otherClass);
	}
};
exports.createChildClassWithArgFromTs = function createChildClassWithArgFromTs(arg) {
	return new exports.ChildClass(arg);
};
exports.createChildClassWithMainFromTs = function createChildClassWithMainFromTs() {
	return new exports.ChildClass(exports.MainClass);
};
exports.childClassWithMainInstance = new exports.ChildClass(exports.MainClass);
exports.createChildClassWithAnonymousFromTs = function createChildClassWithAnonymousFromTs() {
	return new exports.ChildClass(exports.MainClass);
};
exports.childClassWithAnonymousInstance = new exports.ChildClass(exports.MainClass);
exports.createChildClassWithIIFEFromTs = function createChildClassWithIIFEFromTs() {
	return new exports.ChildClass(exports.IIFEClass);
};
exports.childClassWithIIFEInstance = new exports.ChildClass(exports.IIFEClass);
exports.AnonymousClassCreateClass = class {
	constructor(otherClass) {
		this._otherClass = new otherClass(tsInt);
	}
};
exports.createAnonymousClassCreateClassWithArgFromTs = function createAnonymousClassCreateClassWithArgFromTs(arg) {
	return new exports.AnonymousClassCreateClass(arg);
};
exports.createAnonymousClassCreateClassFromTs = function createAnonymousClassCreateClassFromTs() {
	return new exports.AnonymousClassCreateClass(exports.MainClass);
};
exports.createAnonymousClassCreateIIFEClassFromTs = function createAnonymousClassCreateIIFEClassFromTs() {
	return new exports.AnonymousClassCreateClass(exports.IIFEClass);
};
exports.anonymousClassCreateMainInstance = new exports.AnonymousClassCreateClass(exports.MainClass);
exports.anonymousClassCreateIIFEInstance = new exports.AnonymousClassCreateClass(exports.IIFEClass);
exports.IIFECreateClassMain = (function (ctor, value) {
	return new ctor(value);
})(exports.MainClass, tsInt);
exports.IIFECreateClassAnonymous = (function (ctor, value) {
	return new ctor(value);
})(exports.AnonymousClass, tsInt);
exports.IIFECreateClass = (function (ctor, value) {
	return new ctor(value);
})(exports.IIFEClass, tsInt);
exports.MethodClass = class MethodClass {
	init(anyClass, value) {
		return new anyClass(value);
	}
};
exports.createClassFunction = function createClassFunction(arg, val) {
	return new arg(val);
};
exports.createClassArrowFunction = function createClassArrowFunction(arg, val) {
	return new arg(val);
};
exports.checkInstance = function checkInstance(MainClass, instance) {
	if (typeof MainClass !== 'function' || typeof instance !== 'object' || instance === null) {
		throw new TypeError('must be a class');
	}
	return instance instanceof MainClass;
};
