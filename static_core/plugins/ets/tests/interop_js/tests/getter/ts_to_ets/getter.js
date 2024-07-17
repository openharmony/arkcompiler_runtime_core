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
exports.subsetByValueInstanceClass = exports.create_subset_by_value_getter_class_from_ts = exports.SubsetByValueClass = exports.subsetByRefInstanceClass = exports.create_subset_by_ref_getter_class_from_ts = exports.SubsetByRef = exports.anyTypeExplicitGetterInstanceClass = exports.anyTypeGetterInstanceClass = exports.create_any_type_getter_class_from_ts = exports.AnyTypeClass = exports.tupleTypeGetterInstanceClass = exports.create_tuple_type_getter_class_from_ts = exports.TupleTypeClass = exports.literalTypeGetterInstanceClassString = exports.literalTypeGetterInstanceClassInt = exports.create_literal_type_getter_class_from_ts = exports.LiteralClass = exports.unionTypeGetterInstanceClassString = exports.unionTypeGetterInstanceClassInt = exports.create_union_type_getter_class_from_ts = exports.UnionTypeClass = exports.privateGetterInstanceClass = exports.create_private_getter_class_from_ts = exports.PrivateGetterClass = exports.protectedGetterInstanceInheritanceClass = exports.create_protected_getter_inheritance_class_from_ts = exports.ProtectedGetterInheritanceClass = exports.protectedGetterOrigenInstanceClass = exports.create_protected_getter_origen_class_from_ts = exports.ProtectedGetterOrigenClass = exports.publicGetterInstanceClass = exports.create_public_getter_class_from_ts = exports.PublicGetterClass = exports.ts_number = exports.ts_string = void 0;
exports.ts_string = 'string';
exports.ts_number = 1;
var PublicGetterClass = /** @class */ (function () {
    function PublicGetterClass() {
        this._value = exports.ts_string;
    }
    Object.defineProperty(PublicGetterClass.prototype, "value", {
        get: function () {
            return this._value;
        },
        enumerable: false,
        configurable: true
    });
    return PublicGetterClass;
}());
exports.PublicGetterClass = PublicGetterClass;
function create_public_getter_class_from_ts() {
    return new PublicGetterClass();
}
exports.create_public_getter_class_from_ts = create_public_getter_class_from_ts;
exports.publicGetterInstanceClass = new PublicGetterClass();
var ProtectedGetterOrigenClass = /** @class */ (function () {
    function ProtectedGetterOrigenClass() {
        this._value = exports.ts_string;
    }
    Object.defineProperty(ProtectedGetterOrigenClass.prototype, "value", {
        get: function () {
            return this._value;
        },
        enumerable: false,
        configurable: true
    });
    return ProtectedGetterOrigenClass;
}());
exports.ProtectedGetterOrigenClass = ProtectedGetterOrigenClass;
function create_protected_getter_origen_class_from_ts() {
    return new ProtectedGetterOrigenClass();
}
exports.create_protected_getter_origen_class_from_ts = create_protected_getter_origen_class_from_ts;
exports.protectedGetterOrigenInstanceClass = new ProtectedGetterOrigenClass();
var ProtectedGetterInheritanceClass = /** @class */ (function (_super) {
    __extends(ProtectedGetterInheritanceClass, _super);
    function ProtectedGetterInheritanceClass() {
        return _super !== null && _super.apply(this, arguments) || this;
    }
    return ProtectedGetterInheritanceClass;
}(ProtectedGetterOrigenClass));
exports.ProtectedGetterInheritanceClass = ProtectedGetterInheritanceClass;
function create_protected_getter_inheritance_class_from_ts() {
    return new ProtectedGetterInheritanceClass();
}
exports.create_protected_getter_inheritance_class_from_ts = create_protected_getter_inheritance_class_from_ts;
exports.protectedGetterInstanceInheritanceClass = new ProtectedGetterInheritanceClass();
var PrivateGetterClass = /** @class */ (function () {
    function PrivateGetterClass() {
        this._value = exports.ts_string;
    }
    Object.defineProperty(PrivateGetterClass.prototype, "value", {
        get: function () {
            return this._value;
        },
        enumerable: false,
        configurable: true
    });
    return PrivateGetterClass;
}());
exports.PrivateGetterClass = PrivateGetterClass;
function create_private_getter_class_from_ts() {
    return new PrivateGetterClass();
}
exports.create_private_getter_class_from_ts = create_private_getter_class_from_ts;
exports.privateGetterInstanceClass = new PrivateGetterClass();
var UnionTypeClass = /** @class */ (function () {
    function UnionTypeClass(value) {
        this._value = value;
    }
    Object.defineProperty(UnionTypeClass.prototype, "value", {
        get: function () {
            return this._value;
        },
        enumerable: false,
        configurable: true
    });
    return UnionTypeClass;
}());
exports.UnionTypeClass = UnionTypeClass;
function create_union_type_getter_class_from_ts(arg) {
    return new UnionTypeClass(arg);
}
exports.create_union_type_getter_class_from_ts = create_union_type_getter_class_from_ts;
exports.unionTypeGetterInstanceClassInt = new UnionTypeClass(exports.ts_number);
exports.unionTypeGetterInstanceClassString = new UnionTypeClass(exports.ts_string);
var LiteralClass = /** @class */ (function () {
    function LiteralClass(value) {
        this._value = value;
    }
    Object.defineProperty(LiteralClass.prototype, "value", {
        get: function () {
            return this._value;
        },
        enumerable: false,
        configurable: true
    });
    return LiteralClass;
}());
exports.LiteralClass = LiteralClass;
function create_literal_type_getter_class_from_ts(arg) {
    return new LiteralClass(arg);
}
exports.create_literal_type_getter_class_from_ts = create_literal_type_getter_class_from_ts;
exports.literalTypeGetterInstanceClassInt = new LiteralClass(exports.ts_number);
exports.literalTypeGetterInstanceClassString = new LiteralClass(exports.ts_string);
var TupleTypeClass = /** @class */ (function () {
    function TupleTypeClass(value) {
        this._value = value;
    }
    Object.defineProperty(TupleTypeClass.prototype, "value", {
        get: function () {
            return this._value;
        },
        enumerable: false,
        configurable: true
    });
    return TupleTypeClass;
}());
exports.TupleTypeClass = TupleTypeClass;
function create_tuple_type_getter_class_from_ts(arg) {
    return new TupleTypeClass(arg);
}
exports.create_tuple_type_getter_class_from_ts = create_tuple_type_getter_class_from_ts;
exports.tupleTypeGetterInstanceClass = new TupleTypeClass([exports.ts_number, exports.ts_string]);
var AnyTypeClass = /** @class */ (function () {
    function AnyTypeClass() {
    }
    Object.defineProperty(AnyTypeClass.prototype, "value", {
        get: function () {
            return this._value;
        },
        enumerable: false,
        configurable: true
    });
    return AnyTypeClass;
}());
exports.AnyTypeClass = AnyTypeClass;
function create_any_type_getter_class_from_ts() {
    return new AnyTypeClass();
}
exports.create_any_type_getter_class_from_ts = create_any_type_getter_class_from_ts;
exports.anyTypeGetterInstanceClass = new AnyTypeClass();
exports.anyTypeExplicitGetterInstanceClass = new AnyTypeClass();
var SubsetByRef = /** @class */ (function () {
    function SubsetByRef() {
        this._refClass = new PublicGetterClass();
    }
    Object.defineProperty(SubsetByRef.prototype, "value", {
        get: function () {
            return this._refClass.value;
        },
        enumerable: false,
        configurable: true
    });
    return SubsetByRef;
}());
exports.SubsetByRef = SubsetByRef;
function create_subset_by_ref_getter_class_from_ts() {
    return new SubsetByRef();
}
exports.create_subset_by_ref_getter_class_from_ts = create_subset_by_ref_getter_class_from_ts;
exports.subsetByRefInstanceClass = new SubsetByRef();
var SubsetByValueClass = /** @class */ (function () {
    function SubsetByValueClass(value) {
        this._value = value;
    }
    Object.defineProperty(SubsetByValueClass.prototype, "value", {
        get: function () {
            return this._value;
        },
        enumerable: false,
        configurable: true
    });
    return SubsetByValueClass;
}());
exports.SubsetByValueClass = SubsetByValueClass;
var GClass = new PublicGetterClass();
function create_subset_by_value_getter_class_from_ts() {
    return new SubsetByValueClass(GClass.value);
}
exports.create_subset_by_value_getter_class_from_ts = create_subset_by_value_getter_class_from_ts;
exports.subsetByValueInstanceClass = new SubsetByValueClass(GClass.value);
