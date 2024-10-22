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
'use strict';

let __assign = (this && this.__assign) || function (...args) {
    __assign = Object.assign || function(t) {
        for (let s, i = 1, n = args.length; i < n; i++) {
            s = args[i];
            for (let p in s) {
                if (Object.prototype.hasOwnProperty.call(s, p)) {
                    t[p] = s[p];
                }
            }
        }
        return t;
    };
    return __assign.apply(this, args);
};

let _a;

Object.defineProperty(exports, '__esModule', { value: true });
exports.testObject = exports.genericInterfaceImplementation = exports.GenericInterface = exports.ENUM_VALUE =
    exports.UNDEFINED = exports.NULL_VALUE = exports.BOOLEAN_VALUE = exports.FLOAT_VALUE = exports.INT_VALUE =
    exports.STRING_VALUE = void 0;
exports.STRING_VALUE = 'Panda';
exports.INT_VALUE = Number.MAX_SAFE_INTEGER;
exports.FLOAT_VALUE = Math.PI;
exports.BOOLEAN_VALUE = true;
exports.NULL_VALUE = null;
exports.UNDEFINED = (_a = {}) === null || _a === void 0 ? void 0 : _a.value;

let ENUM_VALUE;
(function (ENUM_VALUE) {
    ENUM_VALUE[ENUM_VALUE.OPTION_ONE = 0] = 'OPTION_ONE';
    ENUM_VALUE[ENUM_VALUE.OPTION_TWO = 1] = 'OPTION_TWO';
    ENUM_VALUE[ENUM_VALUE.OPTION_THREE = 2] = 'OPTION_THREE';
})(ENUM_VALUE || (exports.ENUM_VALUE = ENUM_VALUE = {}));

let TUPLE_VALUE = ['abc', 123];

let genericInterface = /** @class */ (function () {
    function genericInterface() {
        this.getInt = function () {
            return Number(exports.INT_VALUE);
        };
        this.getNegativeInt = function () {
            return Number(exports.INT_VALUE) * -1;
        };
        this.getInfinity = function () {
            return Number.POSITIVE_INFINITY;
        };
        this.getNegativeInfinity = function () {
            return Number.NEGATIVE_INFINITY;
        };
        this.getNanAsNumber = function () {
            return Number.NaN;
        };
        this.getBigInt = function () {
            return BigInt(exports.INT_VALUE);
        };
        this.getFloat = function () {
            return exports.FLOAT_VALUE;
        };
        this.getString = function () {
            return exports.STRING_VALUE;
        };
        this.getBoolean = function () {
            return exports.BOOLEAN_VALUE;
        };
        this.getEnum = function () {
            return ENUM_VALUE;
        };
        this.getNull = function () {
            return exports.NULL_VALUE;
        };
        this.getUndefined = function () {
            return exports.UNDEFINED;
        };
    }
    genericInterface.prototype.getAny = function () {
        let randomValues = [exports.STRING_VALUE, exports.INT_VALUE, exports.FLOAT_VALUE, exports.BOOLEAN_VALUE,
            exports.NULL_VALUE, exports.UNDEFINED, new Array(2).fill(' '), {}];
        let index = Math.round(Math.random() * randomValues.length);
        return randomValues[index];
    };
    genericInterface.prototype.getTuple = function () {
        return ['tuple_item_0', 1];
    };
    genericInterface.prototype.getGeneric = function (arg) {
        return __assign(__assign({}, arg), { extendingProperty: 0 });
    };
    genericInterface.prototype.getFunctionReturningType = function (arg) {
        return function (arg) { return 0; };
    };
    return genericInterface;
}());

exports.GenericInterface = genericInterface;

exports.genericInterfaceImplementation = {
    getAny: function () {
        let randomValues = [
            exports.STRING_VALUE,
            exports.INT_VALUE,
            exports.FLOAT_VALUE,
            exports.BOOLEAN_VALUE,
            exports.NULL_VALUE,
            exports.UNDEFINED,
            new Array(2).fill(' '),
            {}
        ];
        let index = Math.round(Math.random() * randomValues.length);
        return randomValues[index];
    },
    getInt: function () {
        return Number(exports.INT_VALUE);
    },
    getBigInt: function () {
        return BigInt(exports.INT_VALUE);
    },
    getFloat: function () {
        return exports.FLOAT_VALUE;
    },
    getNegativeInt: function () {
        return Number(exports.INT_VALUE) * -1;
    },
    getInfinity: function () {
        return Number.POSITIVE_INFINITY;
    },
    getNegativeInfinity: function () {
        return Number.NEGATIVE_INFINITY;
    },
    getNanAsNumber: function () {
        return Number.NaN;
    },
    getString: function () {
        return exports.STRING_VALUE;
    },
    getBoolean: function () {
        return exports.BOOLEAN_VALUE;
    },
    getTuple: function () {
        return TUPLE_VALUE;
    },
    getGeneric: function (arg) {
        return __assign(__assign({}, arg),
            { extendingProperty: Object.keys(arg !== null && arg !== void 0 ? arg : {}).length * -1 });
    },
    getFunctionReturningType: function (arg) {
        return function () {
            console.log('reached');
            switch (typeof arg) {
                case 'string':
                    return arg.toLowerCase() + arg.toUpperCase();
                case 'bigint':
                    return arg / BigInt(2);
                case 'number':
                    return arg * Math.PI;
                default:
                    return 'undefined';
            }
        };
    },
    getEnum: function () {
        return ENUM_VALUE;
    },
    getNull: function () {
        return exports.NULL_VALUE;
    },
    getUndefined: function () {
        return exports.UNDEFINED;
    }
};

exports.testObject = {
    a: 1
};
