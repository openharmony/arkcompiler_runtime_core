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

import {
   ts_string,
   LiteralClass,
   UnionClass,
   InterfaceClass,
   AbstractClass,
   GClass,
   literalClass,
   unionClass,
   interfaceClass,
   abstractClass,
   generic_function,
   tuple_declared_type,
   explicitly_declared_type,
} from '/plugins/ets/tests/interop_js/tests/generic/ts_to_ets/generic.js';

const strContent = 'ets_test_string';
const numContent = 1;
const boolContent = true;

function check_literal_class_generic(): boolean {
   const literalClass = new LiteralClass(strContent);

   return strContent == literalClass.get();
}

function check_union_class_generic(): boolean {
   const unionClass = new UnionClass(strContent);

   return strContent == unionClass.get();
}

function check_interface_class_generic(): boolean {
   const interfaceClass = new InterfaceClass(strContent);

   return strContent == interfaceClass.get();
}

function check_abstract_class_generic(): boolean {
   const abstractClass = new AbstractClass(strContent);

   return strContent == abstractClass.get();
}

function check_generic_class_generic(): boolean {
   const gClass = new GClass(strContent);

   return strContent == gClass.get();
}

function check_generic_literal_class_object_form_ts(): boolean {
   return ts_string === literalClass.get();
}

function check_generic_union_class_object_form_ts(): boolean {
   return ts_string === unionClass.get();
}

function check_generic_interface_class_object_form_ts(): boolean {
   return ts_string === interfaceClass.get();
}

function check_generic_abstract_class_object_form_ts(): boolean {
   return ts_string === abstractClass.get();
}

function check_generic_function_type_any_string(): boolean {
   return strContent == generic_function(strContent);
}

function check_generic_function_type_any_int(): boolean {
   return numContent == generic_function(numContent);
}

function check_generic_function_type_any_bool(): boolean {
   return boolContent as boolean == generic_function(boolContent) as boolean;
}

function check_generic_function_tuple_type() {
   const arr: [int, string] = [numContent, strContent];
   const result = tuple_declared_type<[int, string]>(arr);
   return arr[0] == result[0] && arr[1] == result[1];
}

function check_generic_explicitly_declared_type(): boolean {
   return ts_string === explicitly_declared_type();
}
