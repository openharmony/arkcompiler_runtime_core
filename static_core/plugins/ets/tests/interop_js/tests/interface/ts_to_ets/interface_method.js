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
Object.defineProperty(exports, "__esModule", { value: true });
exports.withOptionalMethodInstanceClass = exports.withoutOptionalMethodInstanceClass = exports.tupleInstanceClass = exports.subsetByValueInstanceClass = exports.unionTypeMethodInstanceClass = exports.create_interface_class_tuple_type_method_from_ts = exports.TupleTypeMethodClass = exports.optional_arg_array = exports.optional_arg = exports.create_class_without_optional_method = exports.create_class_with_optional_method = exports.WithoutOptionalMethodClass = exports.WithOptionalMethodClass = exports.create_subset_by_value_class_from_ts = exports.SubsetByValueClass = exports.subset_by_ref_interface = exports.create_interface_class_union_type_method = exports.UnionTypeMethodClass = exports.create_interface_class_any_type_method = exports.AnyTypeMethodClass = exports.ts_string = exports.ts_number = void 0;
exports.ts_number = 1;
exports.ts_string = 'string';
var AnyTypeMethodClass = /** @class */ (function () {
    function AnyTypeMethodClass() {
    }
    AnyTypeMethodClass.prototype.get = function (a) {
        return a;
    };
    return AnyTypeMethodClass;
}());
exports.AnyTypeMethodClass = AnyTypeMethodClass;
function create_interface_class_any_type_method() {
    return new AnyTypeMethodClass();
}
exports.create_interface_class_any_type_method = create_interface_class_any_type_method;
var UnionTypeMethodClass = /** @class */ (function () {
    function UnionTypeMethodClass() {
    }
    UnionTypeMethodClass.prototype.get = function (a) {
        return a;
    };
    return UnionTypeMethodClass;
}());
exports.UnionTypeMethodClass = UnionTypeMethodClass;
function create_interface_class_union_type_method() {
    return new AnyTypeMethodClass();
}
exports.create_interface_class_union_type_method = create_interface_class_union_type_method;
function subset_by_ref_interface(obj) {
    return obj.get();
}
exports.subset_by_ref_interface = subset_by_ref_interface;
var UserClass = /** @class */ (function () {
    function UserClass() {
        this.value = 1;
    }
    return UserClass;
}());
var SubsetByValueClass = /** @class */ (function () {
    function SubsetByValueClass() {
    }
    SubsetByValueClass.prototype.get = function () {
        return new UserClass();
    };
    return SubsetByValueClass;
}());
exports.SubsetByValueClass = SubsetByValueClass;
function create_subset_by_value_class_from_ts() {
    return new SubsetByValueClass();
}
exports.create_subset_by_value_class_from_ts = create_subset_by_value_class_from_ts;
var WithOptionalMethodClass = /** @class */ (function () {
    function WithOptionalMethodClass() {
    }
    WithOptionalMethodClass.prototype.getNum = function () {
        return exports.ts_number;
    };
    WithOptionalMethodClass.prototype.getStr = function () {
        return exports.ts_string;
    };
    return WithOptionalMethodClass;
}());
exports.WithOptionalMethodClass = WithOptionalMethodClass;
var WithoutOptionalMethodClass = /** @class */ (function () {
    function WithoutOptionalMethodClass() {
    }
    WithoutOptionalMethodClass.prototype.getStr = function () {
        return exports.ts_string;
    };
    return WithoutOptionalMethodClass;
}());
exports.WithoutOptionalMethodClass = WithoutOptionalMethodClass;
function create_class_with_optional_method() {
    return new WithOptionalMethodClass();
}
exports.create_class_with_optional_method = create_class_with_optional_method;
function create_class_without_optional_method() {
    return new WithoutOptionalMethodClass();
}
exports.create_class_without_optional_method = create_class_without_optional_method;
function optional_arg(arg, optional) {
    if (optional) {
        return { with: arg, without: optional };
    }
    return { with: arg };
}
exports.optional_arg = optional_arg;
function optional_arg_array() {
    var arg = [];
    for (var _i = 0; _i < arguments.length; _i++) {
        arg[_i] = arguments[_i];
    }
    var withOptional = arg[0];
    var withoutOptional = arg[1];
    if (withoutOptional) {
        return { with: withOptional, without: withoutOptional };
    }
    return { with: withOptional };
}
exports.optional_arg_array = optional_arg_array;
var TupleTypeMethodClass = /** @class */ (function () {
    function TupleTypeMethodClass() {
    }
    TupleTypeMethodClass.prototype.get = function (arg) {
        return arg;
    };
    return TupleTypeMethodClass;
}());
exports.TupleTypeMethodClass = TupleTypeMethodClass;
function create_interface_class_tuple_type_method_from_ts() {
    return new TupleTypeMethodClass();
}
exports.create_interface_class_tuple_type_method_from_ts = create_interface_class_tuple_type_method_from_ts;
exports.unionTypeMethodInstanceClass = new UnionTypeMethodClass();
exports.subsetByValueInstanceClass = new SubsetByValueClass();
exports.tupleInstanceClass = new TupleTypeMethodClass();
exports.withoutOptionalMethodInstanceClass = new WithoutOptionalMethodClass();
exports.withOptionalMethodInstanceClass = new WithOptionalMethodClass();
