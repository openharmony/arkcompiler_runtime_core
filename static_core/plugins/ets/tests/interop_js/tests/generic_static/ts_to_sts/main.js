'use strict';
/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
let __extends = (this && this.__extends) || (function () {
    let ExtendStatics = function (d, b) {
        ExtendStatics = Object.setPrototypeOf ||
            ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
            function (d, b) { for (let p in b) { if (Object.prototype.hasOwnProperty.call(b, p)) { d[p] = b[p]; } } };
        return ExtendStatics(d, b);
    };
    return function (d, b) {
        if (typeof b !== 'function' && b !== null) {
            throw new TypeError('Class extends value ' + String(b) + ' is not a constructor or null'); }
        ExtendStatics(d, b);
        function ConstrFoo() { this.constructor = d; }
        d.prototype = b === null ? Object.create(b) : (ConstrFoo.prototype = b.prototype, new ConstrFoo());
    };
})();
Object.defineProperty(exports, '__esModule', { value: true });
exports.InterfaceStatic = exports.checkInstance = exports.UserClassStatic = exports.User = 
exports.UnionStatic = exports.SubsetByValueStatic = exports.SubsetByRefStatic = 
exports.ExtraSetStatic = exports.LiteralStatic = exports.GenericExtends = exports.GenericStatic = 
exports.user = exports.BaseObj = exports.tsTuple = exports.tsLiteralStop = exports.tsLiteralStart = 
exports.tsArr = exports.tsBool = exports.tsString = exports.tsInt = void 0;
exports.genericStaticMethodCallFromTs = genericStaticMethodCallFromTs;
exports.genericClassExtendsCallFromTs = genericClassExtendsCallFromTs;
exports.genericLiteralCallFromTs = genericLiteralCallFromTs;
exports.genericExtraSetCallFromTs = genericExtraSetCallFromTs;
exports.genericSubsetByRefStaticCallFromTs = genericSubsetByRefStaticCallFromTs;
exports.genericSubsetByValueStaticCallFromTs = genericSubsetByValueStaticCallFromTs;
exports.genericUnionStaticCallFromTs = genericUnionStaticCallFromTs;
exports.userClassFromTs = userClassFromTs;
exports.userInterfaceFromTs = userInterfaceFromTs;
exports.tsInt = 1;
exports.tsString = 'str';
exports.tsBool = true;
exports.tsArr = [exports.tsInt];
exports.tsLiteralStart = 'start';
exports.tsLiteralStop = 'stop';
exports.tsTuple = [exports.tsInt, exports.tsBool];
exports.BaseObj = { a: exports.tsInt };
exports.user = { tsInt: exports.tsInt };
let GenericStatic = /** @class */ (function () {
    function genericStatic() {
    }
    genericStatic.get = function (value) {
        return value;
    };
    return genericStatic;
}());
exports.GenericStatic = GenericStatic;
function genericStaticMethodCallFromTs(arg) {
    return GenericStatic.get(arg);
}
let GenericExtends = /** @class */ (function (_super) {
    __extends(genericExtends, _super);
    function genericExtends(...args) {
        return _super !== null && _super.apply(this, args) || this;
    }
    return genericExtends;
}(GenericStatic));
exports.GenericExtends = GenericExtends;
;
function genericClassExtendsCallFromTs(arg) {
    return GenericExtends.get(arg);
}
let LiteralStatic = /** @class */ (function () {
    function literalStatic() {
    }
    literalStatic.get = function (value) {
        return value;
    };
    return literalStatic;
}());
exports.LiteralStatic = LiteralStatic;
function genericLiteralCallFromTs(arg) {
    return LiteralStatic.get(arg);
}
let ExtraSetStatic = /** @class */ (function () {
    function extraSetStatic() {
    }
    extraSetStatic.get = function (value) {
        return value[0];
    };
    return extraSetStatic;
}());
exports.ExtraSetStatic = ExtraSetStatic;
function genericExtraSetCallFromTs(arg) {
    return ExtraSetStatic.get(arg);
}
let SubsetByRefStatic = /** @class */ (function () {
    function subsetByRefStatic() {
    }
    subsetByRefStatic.get = function (value) {
        return value;
    };
    return subsetByRefStatic;
}());
exports.SubsetByRefStatic = SubsetByRefStatic;
function genericSubsetByRefStaticCallFromTs(arg) {
    return SubsetByRefStatic.get(arg);
}
let SubsetByValueStatic = /** @class */ (function () {
    function subsetByValueStatic() {
    }
    subsetByValueStatic.get = function (value) {
        return value;
    };
    return subsetByValueStatic;
}());
exports.SubsetByValueStatic = SubsetByValueStatic;
function genericSubsetByValueStaticCallFromTs(arg) {
    return SubsetByValueStatic.get(arg);
}
let UnionStatic = /** @class */ (function () {
    function unionStatic() {
    }
    unionStatic.get = function (value) {
        return value;
    };
    return unionStatic;
}());
exports.UnionStatic = UnionStatic;
function genericUnionStaticCallFromTs(arg) {
    return UnionStatic.get(arg);
}
let User = /** @class */ (function () {
    function user(value) {
        this.value = value;
    }
    return user;
}());
exports.User = User;
let UserClassStatic = /** @class */ (function () {
    function userClassStatic() {
    }
    userClassStatic.get = function (value) {
        return value;
    };
    return userClassStatic;
}());
exports.UserClassStatic = UserClassStatic;
let checkInstance = function (a, b) {
    return a instanceof b;
};
exports.checkInstance = checkInstance;
function userClassFromTs(arg) {
    return UserClassStatic.get(arg);
}
let InterfaceStatic = /** @class */ (function () {
    function interfaceStatic() {
    }
    interfaceStatic.get = function (value) {
        return value;
    };
    return interfaceStatic;
}());
exports.InterfaceStatic = InterfaceStatic;
function userInterfaceFromTs(arg) {
    return InterfaceStatic.get(arg);
}
