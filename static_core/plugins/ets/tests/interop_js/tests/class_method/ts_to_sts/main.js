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
    let extendStatics = function (d, b) {
        extendStatics = Object.setPrototypeOf ||
            ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
            function (d, b) { for (let p in b) { if (Object.prototype.hasOwnProperty.call(b, p)) { d[p] = b[p]; } } };
        return extendStatics(d, b);
    };
    return function (d, b) {
        if (typeof b !== 'function' && b !== null) {
            throw new TypeError('Class extends value ' + String(b) + ' is not a constructor or null'); }
        extendStatics(d, b);
        function ConstrFoo() { this.constructor = d; }
        d.prototype = b === null ? Object.create(b) : (ConstrFoo.prototype = b.prototype, new ConstrFoo());
    };
})();
Object.defineProperty(exports, '__esModule', { value: true });
exports.instanceChildAbstractClass = exports.ChildAbstractClass = exports.instanceChildProtectedClass = 
exports.ChildProtectedClass = exports.instanceProtectedClass = exports.ProtectedClass = 
exports.instancePrivateClass = exports.PrivateClass = exports.StaticClass = 
exports.instanceInterfaceClass = exports.InterfaceClass = exports.instanceChildClass = 
exports.ChildClass = exports.instanceUserClass = exports.UserClass = exports.tsInt = void 0;
exports.createUserClassFromTs = createUserClassFromTs;
exports.createChildClassFromTs = createChildClassFromTs;
exports.createInterfaceClassFromTs = createInterfaceClassFromTs;
exports.createStaticClassFromTs = createStaticClassFromTs;
exports.createPrivateClassFromTs = createPrivateClassFromTs;
exports.createProtectedClassFromTs = createProtectedClassFromTs;
exports.createChildProtectedClassFromTs = createChildProtectedClassFromTs;
exports.createChildAbstractClassFromTs = createChildAbstractClassFromTs;
exports.tsInt = 1;
let UserClass = /** @class */ (function () {
    function UserClass() {
        this.value = exports.tsInt;
    }
    UserClass.prototype.get = function () {
        return this.value;
    };
    UserClass.prototype.compare = function (x) {
        if (typeof x === 'function' && x === this.get) {
            return true;
        }
        return false;
    };
    return UserClass;
}());
exports.UserClass = UserClass;
function createUserClassFromTs() {
    return new UserClass();
}
exports.instanceUserClass = new UserClass();
let ChildClass = /** @class */ (function (_super) {
    __extends(ChildClass, _super);
    function ChildClass(...arg) {
        return _super !== null && _super.apply(this, arg) || this;
    }
    ChildClass.prototype.childMethodCompare = function (x) {
        if (typeof x === 'function' && x === this.get) {
            return true;
        }
        return false;
    };
    return ChildClass;
}(UserClass));
exports.ChildClass = ChildClass;
;
function createChildClassFromTs() {
    return new ChildClass();
}
exports.instanceChildClass = new ChildClass();
let InterfaceClass = /** @class */ (function () {
    function InterfaceClass() {
        this.value = exports.tsInt;
    }
    InterfaceClass.prototype.get = function () {
        return this.value;
    };
    InterfaceClass.prototype.compare = function (x) {
        if (typeof x === 'function' && x === this.get) {
            return true;
        }
        return false;
    };
    return InterfaceClass;
}());
exports.InterfaceClass = InterfaceClass;
function createInterfaceClassFromTs() {
    return new InterfaceClass();
}
exports.instanceInterfaceClass = new InterfaceClass();
let StaticClass = /** @class */ (function () {
    function staticClass() {
    }
    staticClass.get = function () {
        return this.value;
    };
    staticClass.compare = function (x) {
        if (typeof x === 'function' && x === this.get) {
            return true;
        }
        return false;
    };
    staticClass.value = exports.tsInt;
    return staticClass;
}());
exports.StaticClass = StaticClass;
function createStaticClassFromTs() {
    return StaticClass;
}
let PrivateClass = /** @class */ (function () {
    function PrivateClass() {
        this.value = exports.tsInt;
    }
    PrivateClass.prototype.get = function () {
        return this.value;
    };
    PrivateClass.prototype.compare = function (x) {
        if (typeof x === 'function' && x === this.get) {
            return true;
        }
        return false;
    };
    return PrivateClass;
}());
exports.PrivateClass = PrivateClass;
function createPrivateClassFromTs() {
    return new PrivateClass();
}
exports.instancePrivateClass = new PrivateClass();
let ProtectedClass = /** @class */ (function () {
    function ProtectedClass() {
        this.value = exports.tsInt;
    }
    ProtectedClass.prototype.get = function () {
        return this.value;
    };
    ProtectedClass.prototype.compare = function (x) {
        if (typeof x === 'function' && x === this.get) {
            return true;
        }
        return false;
    };
    return ProtectedClass;
}());
exports.ProtectedClass = ProtectedClass;
function createProtectedClassFromTs() {
    return new ProtectedClass();
}
exports.instanceProtectedClass = new ProtectedClass();
let ChildProtectedClass = /** @class */ (function (_super) {
    __extends(ChildProtectedClass, _super);
    function ChildProtectedClass(...arg) {
        return _super !== null && _super.apply(this, arg) || this;
    }
    return ChildProtectedClass;
}(ProtectedClass));
exports.ChildProtectedClass = ChildProtectedClass;
function createChildProtectedClassFromTs() {
    return new ChildProtectedClass();
}
exports.instanceChildProtectedClass = new ProtectedClass();
let AbstractClass = /** @class */ (function () {
    function abstractClass() {
        this.value = exports.tsInt;
    }
    abstractClass.prototype.get = function () {
        return this.value;
    };
    abstractClass.prototype.compare = function (x) {
        if (typeof x === 'function' && x === this.get) {
            return true;
        }
        return false;
    };
    return abstractClass;
}());
let ChildAbstractClass = /** @class */ (function (_super) {
    __extends(ChildAbstractClass, _super);
    function ChildAbstractClass(...arg) {
        return _super !== null && _super.apply(this, arg) || this;
    }
    return ChildAbstractClass;
}(AbstractClass));
exports.ChildAbstractClass = ChildAbstractClass;
;
function createChildAbstractClassFromTs() {
    return new ChildAbstractClass();
}
exports.instanceChildAbstractClass = new ChildAbstractClass();
