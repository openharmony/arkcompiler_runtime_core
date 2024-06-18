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

const num = 1;
const string = 'string';
const bool = true;
const arr = [];
const obj = {};

function isObject(variable) {
  return variable !== null && typeof variable === 'object' && !Array.isArray(variable);
}

const AnyTypeMethodClass = etsVm.getClass("Linterface_method/test/AnyTypeMethodClass;");
const create_interface_class_any_type_method = etsVm.getFunction("Linterface_method/test/ETSGLOBAL;", "create_interface_class_any_type_method");

const UnionTypeMethodClass = etsVm.getClass("Linterface_method/test/UnionTypeMethodClass;");
const create_interface_class_union_type_method = etsVm.getFunction("Linterface_method/test/ETSGLOBAL;", "create_interface_class_union_type_method");

// NOTE: issue (1835) fix Literal type
// const LiteralValueMethodClass = etsVm.getClass("Linterface_method/test/LiteralValueMethodClass;");
// const create_interface_literal_value_class_from_ets = etsVm.getFunction("Linterface_method/test/ETSGLOBAL;", "create_interface_literal_value_class_from_ets");

const TupleTypeMethodClass = etsVm.getClass("Linterface_method/test/TupleTypeMethodClass;");
const create_interface_class_tuple_type_method = etsVm.getFunction("Linterface_method/test/ETSGLOBAL;", "create_interface_class_tuple_type_method");

const SubsetByRefClass = etsVm.getClass("Linterface_method/test/SubsetByRefClass;");
const subset_by_ref_interface = etsVm.getFunction("Linterface_method/test/ETSGLOBAL;", "subset_by_ref_interface");
const call_subset_by_ref_interface_from_ets = etsVm.getFunction("Linterface_method/test/ETSGLOBAL;", "call_subset_by_ref_interface_from_ets");

const SubsetByValueClass = etsVm.getClass("Linterface_method/test/SubsetByValueClass;");
const create_subset_by_value_class_from_ets = etsVm.getFunction("Linterface_method/test/ETSGLOBAL;", "create_subset_by_value_class_from_ets");

// NOTE (issues 17769) fix optional method in interface
// const WithOptionalMethodClass = etsVm.getClass("Linterface_method/test/WithOptionalMethodClass;");
// const create_class_with_optional_method = etsVm.getFunction("Linterface_method/test/ETSGLOBAL;", "create_class_with_optional_method");
// const WithoutOptionalMethodClass = etsVm.getClass("Linterface_method/test/TupleClass;");
// const create_class_without_optional_method = etsVm.getFunction("Linterface_method/test/ETSGLOBAL;", "create_class_without_optional_method");
// const optional_arg = etsVm.getFunction("Linterface_method/test/ETSGLOBAL;", "optional_arg");
// const optional_arg_array = etsVm.getFunction("Linterface_method/test/ETSGLOBAL;", "optional_arg_array");


module.exports = {
  num,
  string,
  bool,
  arr,
  obj,
  isObject,
  AnyTypeMethodClass,
  create_interface_class_any_type_method,
  UnionTypeMethodClass,
  create_interface_class_union_type_method,
  // create_interface_literal_value_class_from_ets,
  // LiteralValueMethodClass,
  SubsetByRefClass,
  subset_by_ref_interface,
  call_subset_by_ref_interface_from_ets,
  SubsetByValueClass,
  create_subset_by_value_class_from_ets,
  TupleTypeMethodClass,
  create_interface_class_tuple_type_method,
  // WithOptionalMethodClass,
  // create_class_with_optional_method,
  // WithoutOptionalMethodClass,
  // create_class_without_optional_method,
  // optional_arg,
  // optional_arg_array,
};
