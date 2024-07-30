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
					for (var p in b) if (Object.prototype.hasOwnProperty.call(b, p)) d[p] = b[p];
				};
			return extendStatics(d, b);
		};
		return function (d, b) {
			if (typeof b !== 'function' && b !== null) {
				throw new TypeError('Class extends value ' + String(b) + ' is not a constructor or null');
			}
			extendStatics(d, b);
			function __() {
				this.constructor = d;
			}
			d.prototype = b === null ? Object.create(b) : ((__.prototype = b.prototype), new __());
		};
	})();
Object.defineProperty(exports, '__esModule', { value: true });
exports.abstractClass =
	exports.interfaceClass =
	exports.unionClass =
	exports.literalClass =
	exports.explicitlyDeclaredType =
	exports.genericSubsetRef =
	exports.tupleDeclaredType =
	exports.genericFunction =
	exports.GClass =
	exports.AbstractClass =
	exports.GAbstract =
	exports.InterfaceClass =
	exports.UnionClass =
	exports.LiteralClass =
	exports.tsNumber =
	exports.tsString =
		void 0;
exports.tsString = 'string';
exports.tsNumber = 1;
var LiteralClass = /** @class */ (function () {
	function LiteralClass(value) {
		this._value = value;
	}
	LiteralClass.prototype.set = function (arg) {
		this._value = arg;
	};
	LiteralClass.prototype.get = function () {
		return this._value;
	};
	return LiteralClass;
})();
exports.LiteralClass = LiteralClass;
var UnionClass = /** @class */ (function () {
	function UnionClass(value) {
		this._value = value;
	}
	UnionClass.prototype.set = function (arg) {
		this._value = arg;
	};
	UnionClass.prototype.get = function () {
		return this._value;
	};
	return UnionClass;
})();
exports.UnionClass = UnionClass;
var InterfaceClass = /** @class */ (function () {
	function InterfaceClass(value) {
		this.value = value;
	}
	InterfaceClass.prototype.set = function (arg) {
		this.value = arg;
	};
	InterfaceClass.prototype.get = function () {
		return this.value;
	};
	return InterfaceClass;
})();
exports.InterfaceClass = InterfaceClass;
var GAbstract = /** @class */ (function () {
	function GAbstract(value) {
		this._value = value;
	}
	GAbstract.prototype.set = function (arg) {
		this._value = arg;
	};
	GAbstract.prototype.get = function () {
		return this._value;
	};
	return GAbstract;
})();
exports.GAbstract = GAbstract;
var AbstractClass = /** @class */ (function (Super) {
	__extends(AbstractClass, Super);
	function AbstractClass(value) {
		return Super.call(this, value) || this;
	}
	return AbstractClass;
})(GAbstract);
exports.AbstractClass = AbstractClass;
var GClass = /** @class */ (function () {
	function GClass(content) {
		this.content = content;
	}
	GClass.prototype.get = function () {
		return this.content;
	};
	return GClass;
})();
exports.GClass = GClass;
function genericFunction(arg) {
	return arg;
}
exports.genericFunction = genericFunction;
function tupleDeclaredType(items) {
	return items;
}
exports.tupleDeclaredType = tupleDeclaredType;
function genericSubsetRef(items) {
	return items;
}
exports.genericSubsetRef = genericSubsetRef;
var explicitlyDeclaredType = function () {
	return exports.tsString;
};
exports.explicitlyDeclaredType = explicitlyDeclaredType;
exports.literalClass = new LiteralClass(exports.tsString);
exports.unionClass = new UnionClass(exports.tsString);
exports.interfaceClass = new InterfaceClass(exports.tsString);
exports.abstractClass = new AbstractClass(exports.tsString);
