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

const etsVm = require("lib/module/ets_interop_js_napi");

const string = 'string';
const number = 1;
const bool = true;
const arr = [number];
const obj = { 'test': number };
const tuple = [number, string];

const check_array = (arg) => arg instanceof Array;
const check_obj = (arg) => arg !== null && typeof arg === 'object' && !Array.isArray(arg);

const PublicGetterClass = etsVm.getClass("Lgetter/test/PublicGetterClass;");
const create_public_getter_class_from_ets = etsVm.getFunction("Lgetter/test/ETSGLOBAL;", "create_public_getter_class_from_ets");

const ProtectedGetterOrigenClass = etsVm.getClass("Lgetter/test/ProtectedGetterOrigenClass;");
const create_protected_getter_origen_class_from_ets = etsVm.getFunction("Lgetter/test/ETSGLOBAL;", "create_protected_getter_origen_class_from_ets");

const ProtectedGetterInheritanceClass = etsVm.getClass("Lgetter/test/ProtectedGetterInheritanceClass;");
const create_protected_getter_inheritance_class_from_ets = etsVm.getFunction("Lgetter/test/ETSGLOBAL;", "create_protected_getter_inheritance_class_from_ets");

const PrivateGetterClass = etsVm.getClass("Lgetter/test/PrivateGetterClass;");
const create_private_getter_class_from_ets = etsVm.getFunction("Lgetter/test/ETSGLOBAL;", "create_private_getter_class_from_ets");

const PrivateGetterInheritanceClass = etsVm.getClass("Lgetter/test/PrivateGetterInheritanceClass;");
const create_private_getter_inheritance_class_from_ets = etsVm.getFunction("Lgetter/test/ETSGLOBAL;", "create_private_getter_inheritance_class_from_ets");

const UnionTypeClass = etsVm.getClass("Lgetter/test/UnionTypeClass;");
const create_union_type_getter_class_from_ets = etsVm.getFunction("Lgetter/test/ETSGLOBAL;", "create_union_type_getter_class_from_ets");

const AnyTypeClass = etsVm.getClass("Lgetter/test/AnyTypeClass;");
const create_any_type_getter_class_from_ets_string = etsVm.getFunction("Lgetter/test/ETSGLOBAL;", "create_any_type_getter_class_from_ets_string");
const create_any_type_getter_class_from_ets_int = etsVm.getFunction("Lgetter/test/ETSGLOBAL;", "create_any_type_getter_class_from_ets_int");
const create_any_type_getter_class_from_ets_bool = etsVm.getFunction("Lgetter/test/ETSGLOBAL;", "create_any_type_getter_class_from_ets_bool");
const create_any_type_getter_class_from_ets_arr = etsVm.getFunction("Lgetter/test/ETSGLOBAL;", "create_any_type_getter_class_from_ets_arr");
const create_any_type_getter_class_from_ets_obj = etsVm.getFunction("Lgetter/test/ETSGLOBAL;", "create_any_type_getter_class_from_ets_obj");
const create_any_type_getter_class_from_ets_tuple = etsVm.getFunction("Lgetter/test/ETSGLOBAL;", "create_any_type_getter_class_from_ets_tuple");
const create_any_type_getter_class_from_ets_union = etsVm.getFunction("Lgetter/test/ETSGLOBAL;", "create_any_type_getter_class_from_ets_union");

const SubsetByRef = etsVm.getClass("Lgetter/test/SubsetByRef;");
const create_subset_by_ref_getter_class_from_ets = etsVm.getFunction("Lgetter/test/ETSGLOBAL;", "create_subset_by_ref_getter_class_from_ets");

const SubsetByValueClass = etsVm.getClass("Lgetter/test/SubsetByValueClass;");
const create_subset_by_value_getter_class_from_ets = etsVm.getFunction("Lgetter/test/ETSGLOBAL;", "create_subset_by_value_getter_class_from_ets");

module.exports = {
    check_array,
    check_obj,
    string,
    number,
    bool,
    arr,
    obj,
    tuple,
    PublicGetterClass,
    create_public_getter_class_from_ets,
    ProtectedGetterOrigenClass,
    create_protected_getter_origen_class_from_ets,
    ProtectedGetterInheritanceClass,
    create_protected_getter_inheritance_class_from_ets,
    PrivateGetterClass,
    create_private_getter_class_from_ets,
    PrivateGetterInheritanceClass,
    create_private_getter_inheritance_class_from_ets,
    UnionTypeClass,
    create_union_type_getter_class_from_ets,
    AnyTypeClass,
    create_any_type_getter_class_from_ets_string,
    create_any_type_getter_class_from_ets_int,
    create_any_type_getter_class_from_ets_bool,
    create_any_type_getter_class_from_ets_arr,
    create_any_type_getter_class_from_ets_obj,
    create_any_type_getter_class_from_ets_tuple,
    create_any_type_getter_class_from_ets_union,
    SubsetByRef,
    create_subset_by_ref_getter_class_from_ets,
    SubsetByValueClass,
    create_subset_by_value_getter_class_from_ets,
}
