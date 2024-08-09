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
exports.SubsetValueSetObject =
	exports.SubsetRefSetObject =
	exports.BaseClassObject =
	exports.SetterAnyTypeObject =
	exports.TupleSetObject =
	exports.InterfaceSetterObject =
	exports.UnionSetterObject =
	exports.AbstractClassObject =
	exports.AbstractClass =
	exports.TupleSet =
	exports.SetterAnyType =
	exports.SubsetValueSet =
	exports.tsTestString =
	exports.SubsetRefSet =
	exports.BaseClass =
	exports.InterfaceSetter =
	exports.UnionSetter =
		void 0;
var UnionSetter = /** @class */ (function () {
	function UnionSetter() {}
	Object.defineProperty(UnionSetter.prototype, 'value', {
		get: function () {
			return this._value;
		},
		set: function (arg) {
			this._value = arg;
		},
		enumerable: false,
		configurable: true,
	});
	return UnionSetter;
})();
exports.UnionSetter = UnionSetter;
var InterfaceSetter = /** @class */ (function () {
	function InterfaceSetter() {}
	Object.defineProperty(InterfaceSetter.prototype, 'value', {
		get: function () {
			return this._value;
		},
		set: function (arg) {
			this._value = arg;
		},
		enumerable: false,
		configurable: true,
	});
	return InterfaceSetter;
})();
exports.InterfaceSetter = InterfaceSetter;
var BaseClass = /** @class */ (function () {
	function BaseClass() {}
	Object.defineProperty(BaseClass.prototype, 'value', {
		get: function () {
			return this._value;
		},
		set: function (arg) {
			this._value = arg;
		},
		enumerable: false,
		configurable: true,
	});
	return BaseClass;
})();
exports.BaseClass = BaseClass;
var SubsetRefSet = /** @class */ (function (_super) {
	__extends(SubsetRefSet, _super);
	function SubsetRefSet() {
		return (_super !== null && _super.apply(this, arguments)) || this;
	}
	return SubsetRefSet;
})(BaseClass);
exports.SubsetRefSet = SubsetRefSet;
exports.tsTestString = 'ts_test_string';
var ValueSetter = /** @class */ (function () {
	function ValueSetter() {
		this._value = exports.tsTestString;
	}
	return ValueSetter;
})();
var SubsetValueSet = /** @class */ (function (_super) {
	__extends(SubsetValueSet, _super);
	function SubsetValueSet() {
		return _super.call(this) || this;
	}
	Object.defineProperty(SubsetValueSet.prototype, 'value', {
		get: function () {
			return this._value;
		},
		set: function (arg) {
			this._value = arg;
		},
		enumerable: false,
		configurable: true,
	});
	return SubsetValueSet;
})(ValueSetter);
exports.SubsetValueSet = SubsetValueSet;
var SetterAnyType = /** @class */ (function () {
	function SetterAnyType() {}
	Object.defineProperty(SetterAnyType.prototype, 'value', {
		get: function () {
			return this._value;
		},
		set: function (arg) {
			this._value = arg;
		},
		enumerable: false,
		configurable: true,
	});
	return SetterAnyType;
})();
exports.SetterAnyType = SetterAnyType;
var TupleSet = /** @class */ (function () {
	function TupleSet() {}
	Object.defineProperty(TupleSet.prototype, 'value', {
		get: function () {
			return this._value;
		},
		set: function (arg) {
			this._value = arg;
		},
		enumerable: false,
		configurable: true,
	});
	return TupleSet;
})();
exports.TupleSet = TupleSet;
var AbstractSetter = /** @class */ (function () {
	function AbstractSetter() {}
	return AbstractSetter;
})();
var AbstractClass = /** @class */ (function (_super) {
	__extends(AbstractClass, _super);
	function AbstractClass() {
		return (_super !== null && _super.apply(this, arguments)) || this;
	}
	Object.defineProperty(AbstractClass.prototype, 'value', {
		get: function () {
			return this._value;
		},
		set: function (arg) {
			this._value = arg;
		},
		enumerable: false,
		configurable: true,
	});
	return AbstractClass;
})(AbstractSetter);
exports.AbstractClass = AbstractClass;
exports.AbstractClassObject = new AbstractClass();
exports.UnionSetterObject = new UnionSetter();
exports.InterfaceSetterObject = new InterfaceSetter();
exports.TupleSetObject = new TupleSet();
exports.SetterAnyTypeObject = new SetterAnyType();
exports.BaseClassObject = new BaseClass();
exports.SubsetRefSetObject = new SubsetRefSet();
exports.SubsetValueSetObject = new SubsetValueSet();
