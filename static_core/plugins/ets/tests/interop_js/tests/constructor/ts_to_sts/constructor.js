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
        function FuC() { this.constructor = d; }
        d.prototype = b === null ? Object.create(b) : (FuC.prototype = b.prototype, new FuC());
    };
})();
Object.defineProperty(exports, '__esModule', { value: true });
exports.childClassInstance = exports.ChildClass = exports.abstractClassInstance = exports.AbstractClass = 
exports.simpleArrowFunction = exports.SimpleObject = exports.methodCreateClassInstance = 
exports.methodCreateAnonymousClass = exports.methodConstructorInstance = exports.MethodCreateConstructor = 
exports.IIFEConstructorInstance = exports.IIFEConstructor = exports.IIFEClassInstance = exports.IIFEClass = 
exports.functionConstructorInstance = exports.anonymousClassInstance = exports.AnonymousClass = exports.namedClassInstance = 
exports.NamedClass = exports.tsInt = void 0;
exports.createNamedClassFromTs = createNamedClassFromTs;
exports.createAnonymousClassFromTs = createAnonymousClassFromTs;
exports.FunctionConstructor = FunctionConstructor;
exports.createFunctionConstructorFromTs = createFunctionConstructorFromTs;
exports.createIIFEClassFromTs = createIIFEClassFromTs;
exports.createIIFEConstructorFromTs = createIIFEConstructorFromTs;
exports.createMethodConstructorClass = createMethodConstructorClass;
exports.createAbstractClassFromTs = createAbstractClassFromTs;
exports.createChildClassFromTs = createChildClassFromTs;
exports.tsInt = 1;
let NamedClass = /** @class */ (function () {
    function NamedClass(value) {
        this._value = value;
    }
    return NamedClass;
}());
exports.NamedClass = NamedClass;
function createNamedClassFromTs() {
    return new NamedClass(exports.tsInt);
}
exports.namedClassInstance = new NamedClass(exports.tsInt);
exports.AnonymousClass = /** @class */ (function () {
    function class1(value) {
        this.value = value;
        this._value = value;
    }
    return class1;
}());
function createAnonymousClassFromTs() {
    return new exports.AnonymousClass(exports.tsInt);
}
exports.anonymousClassInstance = new exports.AnonymousClass(exports.tsInt);
function FunctionConstructor(value) {
    this._value = value;
}
function createFunctionConstructorFromTs() {
    return new FunctionConstructor(exports.tsInt);
}
exports.functionConstructorInstance = new FunctionConstructor(exports.tsInt);
exports.IIFEClass = (function () {
    return /** @class */ (function () {
        function class2(value) {
            this._value = value;
        }
        return class2;
    }());
})();
function createIIFEClassFromTs() {
    return new exports.IIFEClass(exports.tsInt);
}
exports.IIFEClassInstance = new exports.IIFEClass(exports.tsInt);
exports.IIFEConstructor = (function () {
    function constructorFoo(value) {
        this._value = value;
    }
    return constructorFoo;
})();
function createIIFEConstructorFromTs() {
    return new exports.IIFEConstructor(exports.tsInt);
}
exports.IIFEConstructorInstance = new exports.IIFEConstructor(exports.tsInt);
let MethodCreateConstructor = /** @class */ (function () {
    function MethodCreateConstructor() {
    }
    MethodCreateConstructor.prototype.Constructor = function () {
        return /** @class */ (function () {
            function class3(value) {
                this._value = value;
            }
            return class3;
        }());
    };
    return MethodCreateConstructor;
}());
exports.MethodCreateConstructor = MethodCreateConstructor;
function createMethodConstructorClass() {
    return new MethodCreateConstructor();
}
exports.methodConstructorInstance = new MethodCreateConstructor();
exports.methodCreateAnonymousClass = exports.methodConstructorInstance.Constructor();
exports.methodCreateClassInstance = new exports.methodCreateAnonymousClass(exports.tsInt);
exports.SimpleObject = {
    _value: exports.tsInt,
};
let simpleArrowFunction = function (int) {
    return {
        int: int,
    };
};
exports.simpleArrowFunction = simpleArrowFunction;
let Abstract = /** @class */ (function () {
    function abstract(value) {
        this._value = value;
    }
    return abstract;
}());
let AbstractClass = /** @class */ (function (_super) {
    __extends(AbstractClass, _super);
    function AbstractClass(...arg) {
        return _super !== null && _super.apply(this, arg) || this;
    }
    return AbstractClass;
}(Abstract));
exports.AbstractClass = AbstractClass;
function createAbstractClassFromTs() {
    return new AbstractClass(exports.tsInt);
}
exports.abstractClassInstance = new AbstractClass(exports.tsInt);
let ChildClass = /** @class */ (function (_super) {
    __extends(ChildClass, _super);
    function ChildClass(value) {
        return _super.call(this, value) || this;
    }
    return ChildClass;
}(NamedClass));
exports.ChildClass = ChildClass;
function createChildClassFromTs() {
    return new ChildClass(exports.tsInt);
}
exports.childClassInstance = new ChildClass(exports.tsInt);
