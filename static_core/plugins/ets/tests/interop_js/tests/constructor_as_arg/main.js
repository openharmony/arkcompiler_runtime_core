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
exports.check_instance = exports.create_class_arrow_function = exports.create_class_function = exports.MethodClass = exports.IIFECreateClass = exports.IIFECreateClassAnonymous = exports.IIFECreateClassMain = exports.anonymousClassCreateIIFEInstance = exports.anonymousClassCreateMainInstance = exports.create_anonymous_class_create_IIFE_class_from_ts = exports.create_anonymous_class_create_class_from_ts = exports.create_anonymous_class_create_class_with_arg_from_ts = exports.AnonymousClassCreateClass = exports.childClassWithIIFEInstance = exports.create_child_class_with_IIFE_from_ts = exports.childClassWithAnonymousInstance = exports.create_child_class_with_anonymous_from_ts = exports.childClassWithMainInstance = exports.create_child_class_with_main_from_ts = exports.create_child_class_with_arg_from_ts = exports.ChildClass = exports.iifeClassInstance = exports.create_IIFE_class_from_ts = exports.anonymousClassInstance = exports.create_anonymous_class_from_ts = exports.mainClassInstance = exports.create_main_class_from_ts = exports.create_class_with_arg_from_ts = exports.ParentClass = exports.AnonymousClass = exports.IIFEClass = exports.MainClass = exports.ts_int = void 0;
exports.ts_int = 1;
var MainClass = /** @class */ (function () {
    function MainClass(value) {
        this._value = value;
    }
    return MainClass;
}());
exports.MainClass = MainClass;
exports.IIFEClass = (function () {
    return /** @class */ (function () {
        function class_1(value) {
            this._value = value;
        }
        return class_1;
    }());
})();
exports.AnonymousClass = /** @class */ (function () {
    function class_2(value) {
        this._value = value;
    }
    return class_2;
}());
var ParentClass = /** @class */ (function () {
    function ParentClass(other_class) {
        this._other_class = new other_class(exports.ts_int);
    }
    return ParentClass;
}());
exports.ParentClass = ParentClass;
function create_class_with_arg_from_ts(arg) {
    return new ParentClass(arg);
}
exports.create_class_with_arg_from_ts = create_class_with_arg_from_ts;
function create_main_class_from_ts() {
    return new ParentClass(MainClass);
}
exports.create_main_class_from_ts = create_main_class_from_ts;
exports.mainClassInstance = new ParentClass(MainClass);
function create_anonymous_class_from_ts() {
    return new ParentClass(exports.AnonymousClass);
}
exports.create_anonymous_class_from_ts = create_anonymous_class_from_ts;
exports.anonymousClassInstance = new ParentClass(exports.AnonymousClass);
function create_IIFE_class_from_ts() {
    return new ParentClass(exports.IIFEClass);
}
exports.create_IIFE_class_from_ts = create_IIFE_class_from_ts;
exports.iifeClassInstance = new ParentClass(exports.IIFEClass);
var ChildClass = /** @class */ (function (_super) {
    __extends(ChildClass, _super);
    function ChildClass(other_class) {
        return _super.call(this, other_class) || this;
    }
    return ChildClass;
}(ParentClass));
exports.ChildClass = ChildClass;
function create_child_class_with_arg_from_ts(arg) {
    return new ChildClass(arg);
}
exports.create_child_class_with_arg_from_ts = create_child_class_with_arg_from_ts;
function create_child_class_with_main_from_ts() {
    return new ChildClass(MainClass);
}
exports.create_child_class_with_main_from_ts = create_child_class_with_main_from_ts;
exports.childClassWithMainInstance = new ChildClass(MainClass);
function create_child_class_with_anonymous_from_ts() {
    return new ChildClass(MainClass);
}
exports.create_child_class_with_anonymous_from_ts = create_child_class_with_anonymous_from_ts;
exports.childClassWithAnonymousInstance = new ChildClass(MainClass);
function create_child_class_with_IIFE_from_ts() {
    return new ChildClass(exports.IIFEClass);
}
exports.create_child_class_with_IIFE_from_ts = create_child_class_with_IIFE_from_ts;
exports.childClassWithIIFEInstance = new ChildClass(exports.IIFEClass);
exports.AnonymousClassCreateClass = /** @class */ (function () {
    function class_3(other_class) {
        this._other_class = new other_class(exports.ts_int);
    }
    return class_3;
}());
function create_anonymous_class_create_class_with_arg_from_ts(arg) {
    return new exports.AnonymousClassCreateClass(arg);
}
exports.create_anonymous_class_create_class_with_arg_from_ts = create_anonymous_class_create_class_with_arg_from_ts;
function create_anonymous_class_create_class_from_ts() {
    return new exports.AnonymousClassCreateClass(MainClass);
}
exports.create_anonymous_class_create_class_from_ts = create_anonymous_class_create_class_from_ts;
function create_anonymous_class_create_IIFE_class_from_ts() {
    return new exports.AnonymousClassCreateClass(exports.IIFEClass);
}
exports.create_anonymous_class_create_IIFE_class_from_ts = create_anonymous_class_create_IIFE_class_from_ts;
exports.anonymousClassCreateMainInstance = new exports.AnonymousClassCreateClass(MainClass);
exports.anonymousClassCreateIIFEInstance = new exports.AnonymousClassCreateClass(exports.IIFEClass);
exports.IIFECreateClassMain = (function (ctor, value) {
    return new ctor(value);
})(MainClass, exports.ts_int);
exports.IIFECreateClassAnonymous = (function (ctor, value) {
    return new ctor(value);
})(exports.AnonymousClass, exports.ts_int);
exports.IIFECreateClass = (function (ctor, value) {
    return new ctor(value);
})(exports.IIFEClass, exports.ts_int);
var MethodClass = /** @class */ (function () {
    function MethodClass() {
    }
    MethodClass.prototype.init = function (anyClass, value) {
        return new anyClass(value);
    };
    return MethodClass;
}());
exports.MethodClass = MethodClass;
function create_class_function(arg, val) {
    return new arg(val);
}
exports.create_class_function = create_class_function;
function create_class_arrow_function(arg, val) {
    return new arg(val);
}
exports.create_class_arrow_function = create_class_arrow_function;
function check_instance(mainClass, instance) {
    if (typeof mainClass !== 'function' || typeof instance !== 'object' || instance === null) {
        throw new TypeError('must be a class');
    }
    return instance instanceof mainClass;
}
exports.check_instance = check_instance;
