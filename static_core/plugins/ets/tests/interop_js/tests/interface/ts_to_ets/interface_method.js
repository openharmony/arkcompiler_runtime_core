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
Object.defineProperty(exports, '__esModule', { value: true });
exports.withOptionalMethodInstanceClass =
  exports.withoutOptionalMethodInstanceClass =
  exports.tupleInstanceClass =
  exports.subsetByValueInstanceClass =
  exports.unionTypeMethodInstanceClass =
  exports.createInterfaceClassTupleTypeMethodFromTs =
  exports.TupleTypeMethodClass =
  exports.optionalArgArray =
  exports.optionalArg =
  exports.createClassWithoutOptionalMethod =
  exports.createClassWithOptionalMethod =
  exports.WithoutOptionalMethodClass =
  exports.WithOptionalMethodClass =
  exports.createSubsetByValueClassFromTs =
  exports.SubsetByValueClass =
  exports.subsetByRefInterface =
  exports.createInterfaceClassUnionTypeMethod =
  exports.UnionTypeMethodClass =
  exports.createInterfaceClassAnyTypeMethod =
  exports.AnyTypeMethodClass =
  exports.tsString =
  exports.tsNumber =
    void 0;
exports.tsNumber = 1;
exports.tsString = 'string';
var AnyTypeMethodClass = /** @class */ (function () {
	function AnyTypeMethodClass() {}
	AnyTypeMethodClass.prototype.get = function (a) {
		return a;
	};
	return AnyTypeMethodClass;
})();
exports.AnyTypeMethodClass = AnyTypeMethodClass;
function createInterfaceClassAnyTypeMethod() {
	return new AnyTypeMethodClass();
}
exports.createInterfaceClassAnyTypeMethod = createInterfaceClassAnyTypeMethod;
var UnionTypeMethodClass = /** @class */ (function () {
	function UnionTypeMethodClass() {}
	UnionTypeMethodClass.prototype.get = function (a) {
		return a;
	};
	return UnionTypeMethodClass;
})();
exports.UnionTypeMethodClass = UnionTypeMethodClass;
function createInterfaceClassUnionTypeMethod() {
	return new AnyTypeMethodClass();
}
exports.createInterfaceClassUnionTypeMethod =
  createInterfaceClassUnionTypeMethod;
function subsetByRefInterface(obj) {
	return obj.get();
}
exports.subsetByRefInterface = subsetByRefInterface;
var UserClass = /** @class */ (function () {
	function UserClass() {
		this.value = 1;
	}
	return UserClass;
})();
var SubsetByValueClass = /** @class */ (function () {
	function SubsetByValueClass() {}
	SubsetByValueClass.prototype.get = function () {
		return new UserClass();
	};
	return SubsetByValueClass;
})();
exports.SubsetByValueClass = SubsetByValueClass;
function createSubsetByValueClassFromTs() {
	return new SubsetByValueClass();
}
exports.createSubsetByValueClassFromTs = createSubsetByValueClassFromTs;
var WithOptionalMethodClass = /** @class */ (function () {
	function WithOptionalMethodClass() {}
	WithOptionalMethodClass.prototype.getNum = function () {
		return exports.tsNumber;
	};
	WithOptionalMethodClass.prototype.getStr = function () {
		return exports.tsString;
	};
	return WithOptionalMethodClass;
})();
exports.WithOptionalMethodClass = WithOptionalMethodClass;
var WithoutOptionalMethodClass = /** @class */ (function () {
	function WithoutOptionalMethodClass() {}
	WithoutOptionalMethodClass.prototype.getStr = function () {
		return exports.tsString;
	};
	return WithoutOptionalMethodClass;
})();
exports.WithoutOptionalMethodClass = WithoutOptionalMethodClass;
function createClassWithOptionalMethod() {
	return new WithOptionalMethodClass();
}
exports.createClassWithOptionalMethod = createClassWithOptionalMethod;
function createClassWithoutOptionalMethod() {
	return new WithoutOptionalMethodClass();
}
exports.createClassWithoutOptionalMethod = createClassWithoutOptionalMethod;
function optionalArg(arg, optional) {
	if (optional) {
		return { with: arg, without: optional };
	}
	return { with: arg };
}
exports.optionalArg = optionalArg;
function optionalArgArray() {
	var arg = [];
	for (var I = 0; I < arguments.length; I++) {
		arg[I] = arguments[I];
	}
	var withOptional = arg[0];
	var withoutOptional = arg[1];
	if (withoutOptional) {
		return { with: withOptional, without: withoutOptional };
	}
	return { with: withOptional };
}
exports.optionalArgArray = optionalArgArray;
var TupleTypeMethodClass = /** @class */ (function () {
	function TupleTypeMethodClass() {}
	TupleTypeMethodClass.prototype.get = function (arg) {
		return arg;
	};
	return TupleTypeMethodClass;
})();
exports.TupleTypeMethodClass = TupleTypeMethodClass;
function createInterfaceClassTupleTypeMethodFromTs() {
	return new TupleTypeMethodClass();
}
exports.createInterfaceClassTupleTypeMethodFromTs =
  createInterfaceClassTupleTypeMethodFromTs;
exports.unionTypeMethodInstanceClass = new UnionTypeMethodClass();
exports.subsetByValueInstanceClass = new SubsetByValueClass();
exports.tupleInstanceClass = new TupleTypeMethodClass();
exports.withoutOptionalMethodInstanceClass = new WithoutOptionalMethodClass();
exports.withOptionalMethodInstanceClass = new WithOptionalMethodClass();
