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
var __extends =
	(this && this.__extends) ||
	(function () {
		var extendStatics = function (d, b) {
			extendStatics =
				Object.setPrototypeOf ||
				({ __proto__: [] } instanceof Array &&
					function (d, b) {
						d.__proto__ = b;
					}) ||
				function (d, b) {
					for (var p in b) {
						if (Object.prototype.hasOwnProperty.call(b, p)) d[p] = b[p];
					}
				};
			return extendStatics(d, b);
		};
		return function (d, b) {
			if (typeof b !== 'function' && b !== null) throw new TypeError('Class extends value ' + String(b) + ' is not a constructor or null');
			extendStatics(d, b);
			function __() {
				this.constructor = d;
			}
			d.prototype = b === null ? Object.create(b) : ((__.prototype = b.prototype), new __());
		};
	})();
Object.defineProperty(exports, '__esModule', { value: true });
exports.checkInstance =
	exports.createClassArrowFunction =
	exports.createClassFunction =
	exports.MethodClass =
	exports.IIFECreateClass =
	exports.IIFECreateClassAnonymous =
	exports.IIFECreateClassMain =
	exports.anonymousClassCreateIIFEInstance =
	exports.anonymousClassCreateMainInstance =
	exports.createAnonymousClassCreateIIFEClassFromTs =
	exports.createAnonymousClassCreateClassFromTs =
	exports.createAnonymousClassCreateClassWithArgFromTs =
	exports.AnonymousClassCreateClass =
	exports.childClassWithIIFEInstance =
	exports.createChildClassWithIIFEFromTs =
	exports.childClassWithAnonymousInstance =
	exports.createChildClassWithAnonymousFromTs =
	exports.childClassWithMainInstance =
	exports.createChildClassWithMainFromTs =
	exports.createChildClassWithArgFromTs =
	exports.ChildClass =
	exports.iifeClassInstance =
	exports.createIIFEClassFromTs =
	exports.anonymousClassInstance =
	exports.createAnonymousClassFromTs =
	exports.mainClassInstance =
	exports.createMainClassFromTs =
	exports.createClassWithArgFromTs =
	exports.ParentClass =
	exports.AnonymousClass =
	exports.IIFEClass =
	exports.MainClass =
	exports.tsInt =
		void 0;
exports.tsInt = 1;
var MainClass = /** @class */ (function () {
	function MainClass(value) {
		this._value = value;
	}
	return MainClass;
})();
exports.MainClass = MainClass;
exports.IIFEClass = (function () {
	return /** @class */ (function () {
		function class1(value) {
			this._value = value;
		}
		return class1;
	})();
})();
exports.AnonymousClass = /** @class */ (function () {
	function class2(value) {
		this._value = value;
	}
	return class2;
})();
var ParentClass = /** @class */ (function () {
	function ParentClass(otherClass) {
		this.OtherClass = new otherClass(exports.tsInt);
	}
	return ParentClass;
})();
exports.ParentClass = ParentClass;
function createClassWithArgFromTs(arg) {
	return new ParentClass(arg);
}
exports.createClassWithArgFromTs = createClassWithArgFromTs;
function createMainClassFromTs() {
	return new ParentClass(MainClass);
}
exports.createMainClassFromTs = createMainClassFromTs;
exports.mainClassInstance = new ParentClass(MainClass);
function createAnonymousClassFromTs() {
	return new ParentClass(exports.AnonymousClass);
}
exports.createAnonymousClassFromTs = createAnonymousClassFromTs;
exports.anonymousClassInstance = new ParentClass(exports.AnonymousClass);
function createIIFEClassFromTs() {
	return new ParentClass(exports.IIFEClass);
}
exports.createIIFEClassFromTs = createIIFEClassFromTs;
exports.iifeClassInstance = new ParentClass(exports.IIFEClass);
var ChildClass = /** @class */ (function (Super) {
	__extends(ChildClass, Super);
	function ChildClass(otherClass) {
		return Super.call(this, otherClass) || this;
	}
	return ChildClass;
})(ParentClass);
exports.ChildClass = ChildClass;
function createChildClassWithArgFromTs(arg) {
	return new ChildClass(arg);
}
exports.createChildClassWithArgFromTs = createChildClassWithArgFromTs;
function createChildClassWithMainFromTs() {
	return new ChildClass(MainClass);
}
exports.createChildClassWithMainFromTs = createChildClassWithMainFromTs;
exports.childClassWithMainInstance = new ChildClass(MainClass);
function createChildClassWithAnonymousFromTs() {
	return new ChildClass(MainClass);
}
exports.createChildClassWithAnonymousFromTs = createChildClassWithAnonymousFromTs;
exports.childClassWithAnonymousInstance = new ChildClass(MainClass);
function createChildClassWithIIFEFromTs() {
	return new ChildClass(exports.IIFEClass);
}
exports.createChildClassWithIIFEFromTs = createChildClassWithIIFEFromTs;
exports.childClassWithIIFEInstance = new ChildClass(exports.IIFEClass);
exports.AnonymousClassCreateClass = /** @class */ (function () {
	function class3(otherClass) {
		this.OtherClass = new otherClass(exports.tsInt);
	}
	return class3;
})();
function createAnonymousClassCreateClassWithArgFromTs(arg) {
	return new exports.AnonymousClassCreateClass(arg);
}
exports.createAnonymousClassCreateClassWithArgFromTs = createAnonymousClassCreateClassWithArgFromTs;
function createAnonymousClassCreateClassFromTs() {
	return new exports.AnonymousClassCreateClass(MainClass);
}
exports.createAnonymousClassCreateClassFromTs = createAnonymousClassCreateClassFromTs;
function createAnonymousClassCreateIIFEClassFromTs() {
	return new exports.AnonymousClassCreateClass(exports.IIFEClass);
}
exports.createAnonymousClassCreateIIFEClassFromTs = createAnonymousClassCreateIIFEClassFromTs;
exports.anonymousClassCreateMainInstance = new exports.AnonymousClassCreateClass(MainClass);
exports.anonymousClassCreateIIFEInstance = new exports.AnonymousClassCreateClass(exports.IIFEClass);
exports.IIFECreateClassMain = (function (ctor, value) {
	return new ctor(value);
})(MainClass, exports.tsInt);
exports.IIFECreateClassAnonymous = (function (ctor, value) {
	return new ctor(value);
})(exports.AnonymousClass, exports.tsInt);
exports.IIFECreateClass = (function (ctor, value) {
	return new ctor(value);
})(exports.IIFEClass, exports.tsInt);
var MethodClass = /** @class */ (function () {
	function MethodClass() {}
	MethodClass.prototype.init = function (anyClass, value) {
		return new anyClass(value);
	};
	return MethodClass;
})();
exports.MethodClass = MethodClass;
function createClassFunction(arg, val) {
	return new arg(val);
}
exports.createClassFunction = createClassFunction;
function createClassArrowFunction(arg, val) {
	return new arg(val);
}
exports.createClassArrowFunction = createClassArrowFunction;
function checkInstance(mainClass, instance) {
	if (typeof mainClass !== 'function' || typeof instance !== 'object' || instance === null) {
		throw new TypeError('must be a class');
	}
	return instance instanceof mainClass;
}
exports.checkInstance = checkInstance;
