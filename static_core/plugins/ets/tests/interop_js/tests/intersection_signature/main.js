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
        function ConstFoo() { this.constructor = d; }
        d.prototype = b === null ? Object.create(b) : (ConstFoo.prototype = b.prototype, new ConstFoo());
    };
})();
Object.defineProperty(exports, '__esModule', { value: true });
exports.instanceChildAgeNameLiteralClass = exports.ChildAgeNameLiteralClass = 
exports.instanceAgeNameLiteralClass = exports.AgeNameLiteralClass = 
exports.instanceChildAgeNameUnionClass = exports.ChildAgeNameUnionClass = 
exports.instanceAgeNameUnionClass = exports.AgeNameUnionClass = 
exports.instanceChildAgeNameGenericClass = exports.ChildAgeNameGenericClass = 
exports.instanceAgeNameGenericClass = exports.AgeNameGenericClass = 
exports.instanceChildAgeNameInterfaceClass = exports.ChildAgeNameInterfaceClass = 
exports.instanceAgeNameInterfaceClass = exports.AgeNameInterfaceClass = 
exports.tsString = exports.tsNumber = void 0;
exports.createAgeNameClassInterfaceFromTs = createAgeNameClassInterfaceFromTs;
exports.callMethodAgeNameClassInterfaceFromTs = callMethodAgeNameClassInterfaceFromTs;
exports.createChildAgeNameClassInterfaceFromTs = createChildAgeNameClassInterfaceFromTs;
exports.callMethodChildAgeNameClassInterfaceFromTs = callMethodChildAgeNameClassInterfaceFromTs;
exports.checkIntersectionInterface = checkIntersectionInterface;
exports.checkAgeNameGeneric = checkAgeNameGeneric;
exports.createAgeNameClassGenericFromTs = createAgeNameClassGenericFromTs;
exports.callMethodAgeNameClassGenericFromTs = callMethodAgeNameClassGenericFromTs;
exports.createChildAgeNameClassGenericFromTs = createChildAgeNameClassGenericFromTs;
exports.callMethodChildAgeNameClassGenericFromTs = callMethodChildAgeNameClassGenericFromTs;
exports.checkUnionType = checkUnionType;
exports.createAgeNameUnionClassFromTs = createAgeNameUnionClassFromTs;
exports.callMethodAgeNameUnionClassFromTs = callMethodAgeNameUnionClassFromTs;
exports.createChildAgeNameUnionClassFromTs = createChildAgeNameUnionClassFromTs;
exports.callMethodChildAgeNameUnionClassFromTs = callMethodChildAgeNameUnionClassFromTs;
exports.checkLiteralType = checkLiteralType;
exports.createAgeNameLiteralClassFromTs = createAgeNameLiteralClassFromTs;
exports.callMethodAgeNameLiteralClassFromTs = callMethodAgeNameLiteralClassFromTs;
exports.createChildAgeNameLiteralClassFromTs = createChildAgeNameLiteralClassFromTs;
exports.callMethodChildAgeNameLiteralClassFromTs = callMethodChildAgeNameLiteralClassFromTs;
exports.tsNumber = 1;
exports.tsString = 'string';
let AgeNameInterfaceClass = /** @class */ (function () {
    function AgeNameInterfaceClass(name, age) {
        throwExceptionInterface({ name: name, age: age });
        this.name = name;
        this.age = age;
    }
    AgeNameInterfaceClass.prototype.createUser = function (name, age) {
        throwExceptionInterface({ name: name, age: age });
        return new AgeNameInterfaceClass(name, age);
    };
    return AgeNameInterfaceClass;
}());
exports.AgeNameInterfaceClass = AgeNameInterfaceClass;
function createAgeNameClassInterfaceFromTs() {
    new AgeNameInterfaceClass(exports.tsString, exports.tsNumber);
    return true;
}
function callMethodAgeNameClassInterfaceFromTs() {
    let user = new AgeNameInterfaceClass(exports.tsString, exports.tsNumber);
    user.createUser(exports.tsString, exports.tsNumber);
    return true;
}
exports.instanceAgeNameInterfaceClass = new AgeNameInterfaceClass(exports.tsString, exports.tsNumber);
let ChildAgeNameInterfaceClass = /** @class */ (function (_super) {
    __extends(ChildAgeNameInterfaceClass, _super);
    function ChildAgeNameInterfaceClass(...arg) {
        return _super !== null && _super.apply(this, arg) || this;
    }
    return ChildAgeNameInterfaceClass;
}(AgeNameInterfaceClass));
exports.ChildAgeNameInterfaceClass = ChildAgeNameInterfaceClass;
;
function createChildAgeNameClassInterfaceFromTs() {
    new ChildAgeNameInterfaceClass(exports.tsString, exports.tsNumber);
    return true;
}
function callMethodChildAgeNameClassInterfaceFromTs() {
    let user = new ChildAgeNameInterfaceClass(exports.tsString, exports.tsNumber);
    user.createUser(exports.tsString, exports.tsNumber);
    return true;
}
exports.instanceChildAgeNameInterfaceClass = new ChildAgeNameInterfaceClass(exports.tsString, exports.tsNumber);
function checkIntersectionInterface(arg) {
    if (checkInterfaceAge(arg) && checkInterfaceName(arg)) {
        return true;
    }
    ;
    return false;
}
function checkInterfaceAge(object) {
    let key = 'age';
    return key in object && typeof object[key] === 'number';
}
function checkInterfaceName(object) {
    let key = 'name';
    return key in object && typeof object[key] === 'string';
}
function throwExceptionInterface(arg) {
    let check = checkIntersectionInterface(arg);
    if (!check) {
        throw new Error('Invalid input');
    }
}
//<< generic type
function checkAgeNameGeneric(arg) {
    if (checkTypeGenericAge(arg) && checkTypeGenericName(arg)) {
        return true;
    }
    ;
    return false;
}
function checkTypeGenericAge(object) {
    let key = 'age';
    return key in object && typeof object[key] === 'number';
}
function checkTypeGenericName(object) {
    let key = 'name';
    return key in object && typeof object[key] === 'string';
}
let AgeNameGenericClass = /** @class */ (function () {
    function AgeNameGenericClass(name, age) {
        throwExceptionGeneric({ name: name, age: age });
        this.name = name;
        this.age = age;
    }
    AgeNameGenericClass.prototype.createUser = function (name, age) {
        throwExceptionGeneric({ name: name, age: age });
        return new AgeNameInterfaceClass(name, age);
    };
    return AgeNameGenericClass;
}());
exports.AgeNameGenericClass = AgeNameGenericClass;
function createAgeNameClassGenericFromTs() {
    new AgeNameGenericClass(exports.tsString, exports.tsNumber);
    return true;
}
function callMethodAgeNameClassGenericFromTs() {
    let user = new AgeNameGenericClass(exports.tsString, exports.tsNumber);
    user.createUser(exports.tsString, exports.tsNumber);
    return true;
}
exports.instanceAgeNameGenericClass = new AgeNameGenericClass(exports.tsString, exports.tsNumber);
let ChildAgeNameGenericClass = /** @class */ (function (_super) {
    __extends(ChildAgeNameGenericClass, _super);
    function ChildAgeNameGenericClass(...arg) {
        return _super !== null && _super.apply(this, arg) || this;
    }
    return ChildAgeNameGenericClass;
}(AgeNameGenericClass));
exports.ChildAgeNameGenericClass = ChildAgeNameGenericClass;
;
function createChildAgeNameClassGenericFromTs() {
    new ChildAgeNameGenericClass(exports.tsString, exports.tsNumber);
    return true;
}
function callMethodChildAgeNameClassGenericFromTs() {
    let user = new ChildAgeNameGenericClass(exports.tsString, exports.tsNumber);
    user.createUser(exports.tsString, exports.tsNumber);
    return true;
}
exports.instanceChildAgeNameGenericClass = new ChildAgeNameGenericClass(exports.tsString, exports.tsNumber);
function throwExceptionGeneric(arg) {
    let check = checkIntersectionInterface(arg);
    if (!check) {
        throw new Error('Invalid input');
    }
}
function checkUnionType(arg) {
    if (checkTypeAgeUnion(arg) && checkTypeNameUnion(arg)) {
        return true;
    }
    ;
    return false;
}
function checkTypeAgeUnion(object) {
    let key = 'age';
    return key in object && (typeof object[key] === 'number' || typeof object[key] === 'boolean');
}
function checkTypeNameUnion(object) {
    let key = 'name';
    return key in object && (typeof object[key] === 'string' || typeof object[key] === 'number');
}
let AgeNameUnionClass = /** @class */ (function () {
    function AgeNameUnionClass(name, age) {
        throwExceptionUnion({ name: name, age: age });
        this.name = name;
        this.age = age;
    }
    AgeNameUnionClass.prototype.createUser = function (name, age) {
        throwExceptionUnion({ name: name, age: age });
        return new AgeNameUnionClass(name, age);
    };
    return AgeNameUnionClass;
}());
exports.AgeNameUnionClass = AgeNameUnionClass;
function createAgeNameUnionClassFromTs() {
    new AgeNameUnionClass(exports.tsString, exports.tsNumber);
    return true;
}
function callMethodAgeNameUnionClassFromTs() {
    let user = new AgeNameUnionClass(exports.tsString, exports.tsNumber);
    user.createUser(exports.tsString, exports.tsNumber);
    return true;
}
exports.instanceAgeNameUnionClass = new AgeNameUnionClass(exports.tsString, exports.tsNumber);
let ChildAgeNameUnionClass = /** @class */ (function (_super) {
    __extends(ChildAgeNameUnionClass, _super);
    function ChildAgeNameUnionClass(...arg) {
        return _super !== null && _super.apply(this, arg) || this;
    }
    return ChildAgeNameUnionClass;
}(AgeNameUnionClass));
exports.ChildAgeNameUnionClass = ChildAgeNameUnionClass;
;
function createChildAgeNameUnionClassFromTs() {
    new ChildAgeNameUnionClass(exports.tsString, exports.tsNumber);
    return true;
}
function callMethodChildAgeNameUnionClassFromTs() {
    let user = new ChildAgeNameUnionClass(exports.tsString, exports.tsNumber);
    user.createUser(exports.tsString, exports.tsNumber);
    return true;
}
exports.instanceChildAgeNameUnionClass = new ChildAgeNameUnionClass(exports.tsString, exports.tsNumber);
function throwExceptionUnion(arg) {
    let check = checkUnionType(arg);
    if (!check) {
        throw new Error('Invalid input');
    }
}
function checkLiteralType(arg) {
    if (checkTypeAgeLiteral(arg) && checkTypeNameLiteral(arg)) {
        return true;
    }
    ;
    return false;
}
function checkTypeAgeLiteral(object) {
    let key = 'age';
    return key in object && (object[key] === 1 || object[key] === true);
}
function checkTypeNameLiteral(object) {
    let key = 'name';
    return key in object && (object[key] === 'string' || object[key] === 1);
}
let AgeNameLiteralClass = /** @class */ (function () {
    function AgeNameLiteralClass(name, age) {
        throwExceptionLiteral({ name: name, age: age });
        this.name = name;
        this.age = age;
    }
    AgeNameLiteralClass.prototype.createUser = function (name, age) {
        throwExceptionLiteral({ name: name, age: age });
        return new AgeNameLiteralClass(name, age);
    };
    return AgeNameLiteralClass;
}());
exports.AgeNameLiteralClass = AgeNameLiteralClass;
function createAgeNameLiteralClassFromTs() {
    new AgeNameLiteralClass(exports.tsString, exports.tsNumber);
    return true;
}
function callMethodAgeNameLiteralClassFromTs() {
    let user = new AgeNameLiteralClass(exports.tsString, exports.tsNumber);
    user.createUser(exports.tsString, exports.tsNumber);
    return true;
}
exports.instanceAgeNameLiteralClass = new AgeNameLiteralClass(exports.tsString, exports.tsNumber);
let ChildAgeNameLiteralClass = /** @class */ (function (_super) {
    __extends(ChildAgeNameLiteralClass, _super);
    function ChildAgeNameLiteralClass(...arg) {
        return _super !== null && _super.apply(this, arg) || this;
    }
    return ChildAgeNameLiteralClass;
}(AgeNameLiteralClass));
exports.ChildAgeNameLiteralClass = ChildAgeNameLiteralClass;
;
function createChildAgeNameLiteralClassFromTs() {
    new ChildAgeNameLiteralClass(exports.tsString, exports.tsNumber);
    return true;
}
function callMethodChildAgeNameLiteralClassFromTs() {
    let user = new ChildAgeNameLiteralClass(exports.tsString, exports.tsNumber);
    user.createUser(exports.tsString, exports.tsNumber);
    return true;
}
exports.instanceChildAgeNameLiteralClass = new ChildAgeNameLiteralClass(exports.tsString, exports.tsNumber);
function throwExceptionLiteral(arg) {
    let check = checkLiteralType(arg);
    if (!check) {
        throw new Error('Invalid input');
    }
}
