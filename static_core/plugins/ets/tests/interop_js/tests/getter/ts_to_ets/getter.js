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
const __extends =
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
			function AdhocConstruct() {
				this.constructor = d;
			}
			d.prototype = b === null ? Object.create(b) : ((AdhocConstruct.prototype = b.prototype), new AdhocConstruct());
		};
	})();
Object.defineProperty(exports, '__esModule', { value: true });
exports.subsetByValueInstanceClass =
	exports.createSubsetByValueGetterClassFromTs =
	exports.SubsetByValueClass =
	exports.subsetByRefInstanceClass =
	exports.createSubsetByRefGetterClassFromTs =
	exports.SubsetByRef =
	exports.anyTypeExplicitGetterInstanceClass =
	exports.anyTypeGetterInstanceClass =
	exports.createAnyTypeGetterClassFromTs =
	exports.AnyTypeClass =
	exports.tupleTypeGetterInstanceClass =
	exports.createTupleTypeGetterClassFromTs =
	exports.TupleTypeClass =
	exports.literalTypeGetterInstanceClassString =
	exports.literalTypeGetterInstanceClassInt =
	exports.createLiteralTypeGetterClassFromTs =
	exports.LiteralClass =
	exports.unionTypeGetterInstanceClassString =
	exports.unionTypeGetterInstanceClassInt =
	exports.createUnionTypeGetterClassFromTs =
	exports.UnionTypeClass =
	exports.privateGetterInstanceClass =
	exports.createPrivateGetterClassFromTs =
	exports.PrivateGetterClass =
	exports.protectedGetterInstanceInheritanceClass =
	exports.createProtectedGetterInheritanceClassFromTs =
	exports.ProtectedGetterInheritanceClass =
	exports.protectedGetterOrigenInstanceClass =
	exports.createProtectedGetterOrigenClassFromTs =
	exports.ProtectedGetterOrigenClass =
	exports.publicGetterInstanceClass =
	exports.createPublicGetterClassFromTs =
	exports.PublicGetterClass =
	exports.tsNumber =
	exports.tsString =
		void 0;
exports.tsString = 'string';
exports.tsNumber = 1;
const PublicGetterClass = /** @class */ (function () {
	function publicGetterClass() {
		this._value = exports.tsString;
	}
	Object.defineProperty(publicGetterClass.prototype, 'value', {
		get: function () {
			return this._value;
		},
		enumerable: false,
		configurable: true,
	});
	return publicGetterClass;
})();
exports.PublicGetterClass = PublicGetterClass;
function createPublicGetterClassFromTs() {
	return new PublicGetterClass();
}
exports.createPublicGetterClassFromTs = createPublicGetterClassFromTs;
exports.publicGetterInstanceClass = new PublicGetterClass();
const ProtectedGetterOrigenClass = /** @class */ (function () {
	function protectedGetterOrigenClass() {
		this._value = exports.tsString;
	}
	Object.defineProperty(protectedGetterOrigenClass.prototype, 'value', {
		get: function () {
			return this._value;
		},
		enumerable: false,
		configurable: true,
	});
	return protectedGetterOrigenClass;
})();
exports.ProtectedGetterOrigenClass = ProtectedGetterOrigenClass;
function createProtectedGetterOrigenClassFromTs() {
	return new ProtectedGetterOrigenClass();
}
exports.createProtectedGetterOrigenClassFromTs = createProtectedGetterOrigenClassFromTs;
exports.protectedGetterOrigenInstanceClass = new ProtectedGetterOrigenClass();
const ProtectedGetterInheritanceClass = /** @class */ (function (_super) {
	__extends(exports.protectedGetterOrigenInstanceClass, _super);
	function protectedGetterInheritanceClass(...args) {
		return (_super !== null && _super.apply(this, args)) || this;
	}
	return protectedGetterInheritanceClass;
})(ProtectedGetterOrigenClass);
exports.ProtectedGetterInheritanceClass = ProtectedGetterInheritanceClass;
function createProtectedGetterInheritanceClassFromTs() {
	return new ProtectedGetterInheritanceClass();
}
exports.createProtectedGetterInheritanceClassFromTs = createProtectedGetterInheritanceClassFromTs;
exports.protectedGetterInstanceInheritanceClass = new ProtectedGetterInheritanceClass();
const PrivateGetterClass = /** @class */ (function () {
	function privateGetterClass() {
		this._value = exports.tsString;
	}
	Object.defineProperty(privateGetterClass.prototype, 'value', {
		get: function () {
			return this._value;
		},
		enumerable: false,
		configurable: true,
	});
	return privateGetterClass;
})();
exports.PrivateGetterClass = PrivateGetterClass;
function createPrivateGetterClassFromTs() {
	return new PrivateGetterClass();
}
exports.createPrivateGetterClassFromTs = createPrivateGetterClassFromTs;
exports.privateGetterInstanceClass = new PrivateGetterClass();
const UnionTypeClass = /** @class */ (function () {
	function unionTypeClass(value) {
		this._value = value;
	}
	Object.defineProperty(unionTypeClass.prototype, 'value', {
		get: function () {
			return this._value;
		},
		enumerable: false,
		configurable: true,
	});
	return unionTypeClass;
})();
exports.UnionTypeClass = UnionTypeClass;
function createUnionTypeGetterClassFromTs(arg) {
	return new UnionTypeClass(arg);
}
exports.createUnionTypeGetterClassFromTs = createUnionTypeGetterClassFromTs;
exports.unionTypeGetterInstanceClassInt = new UnionTypeClass(exports.tsNumber);
exports.unionTypeGetterInstanceClassString = new UnionTypeClass(exports.tsString);
const LiteralClass = /** @class */ (function () {
	function literalClass(value) {
		this._value = value;
	}
	Object.defineProperty(literalClass.prototype, 'value', {
		get: function () {
			return this._value;
		},
		enumerable: false,
		configurable: true,
	});
	return literalClass;
})();
exports.LiteralClass = LiteralClass;
function createLiteralTypeGetterClassFromTs(arg) {
	return new LiteralClass(arg);
}
exports.createLiteralTypeGetterClassFromTs = createLiteralTypeGetterClassFromTs;
exports.literalTypeGetterInstanceClassInt = new LiteralClass(exports.tsNumber);
exports.literalTypeGetterInstanceClassString = new LiteralClass(exports.tsString);
const TupleTypeClass = /** @class */ (function () {
	function tupleTypeClass(value) {
		this._value = value;
	}
	Object.defineProperty(tupleTypeClass.prototype, 'value', {
		get: function () {
			return this._value;
		},
		enumerable: false,
		configurable: true,
	});
	return tupleTypeClass;
})();
exports.TupleTypeClass = TupleTypeClass;
function createTupleTypeGetterClassFromTs(arg) {
	return new TupleTypeClass(arg);
}
exports.createTupleTypeGetterClassFromTs = createTupleTypeGetterClassFromTs;
exports.tupleTypeGetterInstanceClass = new TupleTypeClass([exports.tsNumber, exports.tsString]);
const AnyTypeClass = /** @class */ (function () {
	function anyTypeClass() {}
	Object.defineProperty(anyTypeClass.prototype, 'value', {
		get: function () {
			return this._value;
		},
		enumerable: false,
		configurable: true,
	});
	return anyTypeClass;
})();
exports.AnyTypeClass = AnyTypeClass;
function createAnyTypeGetterClassFromTs() {
	return new AnyTypeClass();
}
exports.createAnyTypeGetterClassFromTs = createAnyTypeGetterClassFromTs;
exports.anyTypeGetterInstanceClass = new AnyTypeClass();
exports.anyTypeExplicitGetterInstanceClass = new AnyTypeClass();
const SubsetByRef = /** @class */ (function () {
	function subsetByRef() {
		this.RefClass = new PublicGetterClass();
	}
	Object.defineProperty(subsetByRef.prototype, 'value', {
		get: function () {
			return this.RefClass.value;
		},
		enumerable: false,
		configurable: true,
	});
	return subsetByRef;
})();
exports.SubsetByRef = SubsetByRef;
function createSubsetByRefGetterClassFromTs() {
	return new SubsetByRef();
}
exports.createSubsetByRefGetterClassFromTs = createSubsetByRefGetterClassFromTs;
exports.subsetByRefInstanceClass = new SubsetByRef();
const SubsetByValueClass = /** @class */ (function () {
	function subsetByValueClass(value) {
		this._value = value;
	}
	Object.defineProperty(subsetByValueClass.prototype, 'value', {
		get: function () {
			return this._value;
		},
		enumerable: false,
		configurable: true,
	});
	return subsetByValueClass;
})();
exports.SubsetByValueClass = SubsetByValueClass;
const GClass = new PublicGetterClass();
function createSubsetByValueGetterClassFromTs() {
	return new SubsetByValueClass(GClass.value);
}
exports.createSubsetByValueGetterClassFromTs = createSubsetByValueGetterClassFromTs;
exports.subsetByValueInstanceClass = new SubsetByValueClass(GClass.value);
