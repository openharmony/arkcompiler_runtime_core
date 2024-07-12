"use strict";
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
var __extends = (this && this.__extends) || (function () {
    var extendStatics = function (d, b) {
        extendStatics = Object.setPrototypeOf ||
            ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
            function (d, b) { for (var p in b) if (Object.prototype.hasOwnProperty.call(b, p)) d[p] = b[p]; };
        return extendStatics(d, b);
    };
    return function (d, b) {
        if (typeof b !== "function" && b !== null)
            throw new TypeError("Class extends value " + String(b) + " is not a constructor or null");
        extendStatics(d, b);
        function __() { this.constructor = d; }
        d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
    };
})();
Object.defineProperty(exports, "__esModule", { value: true });
exports.abstractClass = exports.interfaceClass = exports.unionClass = exports.literalClass = exports.explicitly_declared_type = exports.generic_subset_ref = exports.tuple_declared_type = exports.generic_function = exports.GClass = exports.AbstractClass = exports.GAbstract = exports.InterfaceClass = exports.UnionClass = exports.LiteralClass = exports.ts_number = exports.ts_string = void 0;
exports.ts_string = 'string';
exports.ts_number = 1;
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
}());
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
}());
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
}());
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
}());
exports.GAbstract = GAbstract;
var AbstractClass = /** @class */ (function (_super) {
    __extends(AbstractClass, _super);
    function AbstractClass(value) {
        return _super.call(this, value) || this;
    }
    return AbstractClass;
}(GAbstract));
exports.AbstractClass = AbstractClass;
var GClass = /** @class */ (function () {
    function GClass(content) {
        this.content = content;
    }
    GClass.prototype.get = function () {
        return this.content;
    };
    return GClass;
}());
exports.GClass = GClass;
function generic_function(arg) {
    return arg;
}
exports.generic_function = generic_function;
function tuple_declared_type(items) {
    return items;
}
exports.tuple_declared_type = tuple_declared_type;
;
function generic_subset_ref(items) {
    return items;
}
exports.generic_subset_ref = generic_subset_ref;
var explicitly_declared_type = function () {
    return exports.ts_string;
};
exports.explicitly_declared_type = explicitly_declared_type;
exports.literalClass = new LiteralClass(exports.ts_string);
exports.unionClass = new UnionClass(exports.ts_string);
exports.interfaceClass = new InterfaceClass(exports.ts_string);
exports.abstractClass = new AbstractClass(exports.ts_string);
