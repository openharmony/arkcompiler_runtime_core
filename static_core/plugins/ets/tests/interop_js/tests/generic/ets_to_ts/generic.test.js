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

const UnionClass = etsVm.getClass("Lgeneric/test/UnionClass;");
const create_union_object_from_ets = etsVm.getFunction("Lgeneric/test/ETSGLOBAL;", "create_union_object_from_ets");

const AbstractClass = etsVm.getClass("Lgeneric/test/AbstractClass;");
const create_abstract_object_from_ets = etsVm.getFunction("Lgeneric/test/ETSGLOBAL;", "create_abstract_object_from_ets");

const GIClass = etsVm.getClass("Lgeneric/test/GIClass;");
const create_interface_object_from_ets = etsVm.getFunction("Lgeneric/test/ETSGLOBAL;", "create_interface_object_from_ets");

const GClass = etsVm.getClass("Lgeneric/test/GClass;");
const create_generic_object_from_ets = etsVm.getFunction("Lgeneric/test/ETSGLOBAL;", "create_generic_object_from_ets");

const generic_function = etsVm.getFunction("Lgeneric/test/ETSGLOBAL;", "generic_function");

const tuple_declared_type = etsVm.getFunction("Lgeneric/test/ETSGLOBAL;", "tuple_declared_type");

const generic_subset_ref = etsVm.getFunction("Lgeneric/test/ETSGLOBAL;", "generic_subset_ref");
// const explicitly_declared_type = etsVm.getFunction("Lgeneric/test/ETSGLOBAL;", "explicitly_declared_type");

const num = 1;
const bool = true;
const data = { data: 'string' };
const str = 'string';


module.exports = {
    UnionClass,
    create_union_object_from_ets,
    GIClass,
    create_interface_object_from_ets,
    GClass,
    create_generic_object_from_ets,
    generic_function,
    tuple_declared_type,
    generic_subset_ref,
    AbstractClass,
    // explicitly_declared_type,
    create_abstract_object_from_ets,
    num,
    bool,
    data,
    str,
}