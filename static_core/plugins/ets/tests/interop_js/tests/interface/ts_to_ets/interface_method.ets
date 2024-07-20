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
    ts_number,
    ts_string,
    AnyTypeMethodClass,
    create_interface_class_any_type_method,
    UnionTypeMethodClass,
    create_interface_class_union_type_method,
    unionTypeMethodInstanceClass,
    subset_by_ref_interface,
    SubsetByValueClass,
    create_subset_by_value_class_from_ts,
    subsetByValueInstanceClass,
    TupleTypeMethodClass,
    create_interface_class_tuple_type_method_from_ts,
    tupleInstanceClass,
    WithOptionalMethodClass,
    WithoutOptionalMethodClass,
    create_class_with_optional_method,
    create_class_without_optional_method,
    withoutOptionalMethodInstanceClass,
    withOptionalMethodInstanceClass,
    optional_arg,
    optional_arg_array,
} from '/plugins/ets/tests/interop_js/tests/interface/ts_to_ets/interface_method.js';

const str = 'string';
const bool = true;
const num = 1;
const arr = [];
const obj: Object = {};
const arrTuple: [int, string] = [num, str];

class ObjMethod {
    get(): int {
        return num;
    }
}

function check_any_type_interface_class_string(): boolean {
    const anyTypeMethodClass = new AnyTypeMethodClass();

    return str as string == anyTypeMethodClass.get(str) as string;
}

function check_any_type_interface_class_int(): boolean {
    const anyTypeMethodClass = new AnyTypeMethodClass();

    return num as int == anyTypeMethodClass.get(num) as int;
}

function check_any_type_interface_class_bool(): boolean {
    const anyTypeMethodClass = new AnyTypeMethodClass();

    return bool as boolean == anyTypeMethodClass.get(bool) as boolean;
}

function check_any_type_interface_class_array(): boolean {
    const anyTypeMethodClass = new AnyTypeMethodClass();

    return anyTypeMethodClass.get(arr) as Array<int> instanceof Array;
}

function check_any_type_interface_class_object(): boolean {
    const anyTypeMethodClass = new AnyTypeMethodClass();

    return 'object' == typeof anyTypeMethodClass.get(obj) as Object;
}

function check_create_interface_class_any_type_method_string(): boolean {
    const anyTypeMethodClass = create_interface_class_any_type_method();

    return str as string == anyTypeMethodClass.get(str) as string;
}

function check_create_interface_class_any_type_method_int(): boolean {
    const anyTypeMethodClass = create_interface_class_any_type_method();

    return num as int == anyTypeMethodClass.get(num) as int;
}

function check_create_interface_class_any_type_method_bool(): boolean {
    const anyTypeMethodClass = create_interface_class_any_type_method();

    return bool as boolean == anyTypeMethodClass.get(bool) as boolean;
}

function check_create_interface_class_any_type_method_array(): boolean {
    const anyTypeMethodClass = create_interface_class_any_type_method();

    return anyTypeMethodClass.get(arr) as Array<int> instanceof Array;
}

function check_create_interface_class_any_type_method_object(): boolean {
    const anyTypeMethodClass = create_interface_class_any_type_method();

    return 'object' == typeof anyTypeMethodClass.get(obj) as Object;
}

function check_union_type_interface_class_string(): boolean {
    const unionTypeMethodClass = new UnionTypeMethodClass();

    return str as string == unionTypeMethodClass.get(str) as string;
}

function check_union_type_interface_class_int(): boolean {
    const unionTypeMethodClass = new UnionTypeMethodClass();

    return num as int == unionTypeMethodClass.get(num) as int;
}

function check_create_interface_class_union_type_method_string(): boolean {
    const unionTypeMethodClass = create_interface_class_union_type_method();

    return str as string == unionTypeMethodClass.get(str) as string;
}

function check_create_interface_class_union_type_method_int(): boolean {
    const unionTypeMethodClass = create_interface_class_union_type_method();

    return num as int == unionTypeMethodClass.get(num) as int;
}

function check_instance_interface_class_union_type_method_string(): boolean {
    return str as string == unionTypeMethodInstanceClass.get(str) as string;
}

function check_instance_interface_class_union_type_method_int(): boolean {
    return num as int == unionTypeMethodInstanceClass.get(num) as int;
}

function check_subset_by_ref_interface(): boolean {
    return num as int == subset_by_ref_interface(new ObjMethod()) as int;
}

function check_subset_by_ref_interface_error(): boolean {
    try {
        subset_by_ref_interface(obj);

        return false;
    } catch (e) {
        return true;
    }
}

function check_subset_by_value_class(): boolean {
    const subsetByValueClass = new SubsetByValueClass();
    const userClass = subsetByValueClass.get();

    return ts_number as int == userClass.value as int;
}

function check_create_subset_by_value_class_from_ts(): boolean {
    const subsetByValueClass = create_subset_by_value_class_from_ts();
    const userClass = subsetByValueClass.get();

    return ts_number as int == userClass.value as int;
}

function check_instance_subset_by_value_class_from_ts(): boolean {
    const userClass = subsetByValueInstanceClass.get();

    return ts_number as int == userClass.value as int;
}

function check_tuple_type_class(): boolean {
    const tupleClass = new TupleTypeMethodClass();

    const result = tupleClass.get(arrTuple);
    return result as Array<[int, string]> instanceof Array
        && result[0] as int == num as int
        && result[1] as string == str as string;
}

function check_create_tuple_class_from_ts(): boolean {
    const tupleClass = create_interface_class_tuple_type_method_from_ts();

    const result = tupleClass.get(arrTuple);
    return result as Array<[int, string]> instanceof Array
        && result[0] as int == num as int
        && result[1] as string == str as string;
}

function check_instance_tuple_class_from_ts(): boolean {
    const result = tupleInstanceClass.get(arrTuple);
    return result as Array<[int, string]> instanceof Array
        && result[0] as int == num as int
        && result[1] as string == str as string;
}

function check_with_optional_method_class(): boolean {
    const optionalClass = new WithOptionalMethodClass();

    return optionalClass.getNum() as int == ts_number as int
        && optionalClass.getStr() as string == ts_string as string;
}

function check_without_optional_method_class(): boolean {
    const optionalClass = new WithoutOptionalMethodClass();

    return optionalClass.getStr() as string == ts_string as string;
}

function check_create_class_with_optional_method() {
    const optionalClass = create_class_with_optional_method();

    return optionalClass.getNum() as int == ts_number as int
        && optionalClass.getStr() as string == ts_string as string;
}

function check_create_class_without_optional_method() {
    const optionalClass = create_class_without_optional_method();

    return optionalClass.getStr() as string == ts_string as string;
}

function check_with_optional_method_instance_class(): boolean {
    return withOptionalMethodInstanceClass.getNum() as int == ts_number as int
        && withOptionalMethodInstanceClass.getStr() as string == ts_string as string;
}

function check_without_optional_method_instance_class() {
    return withoutOptionalMethodInstanceClass.getStr() as string == ts_string as string;
}

function check_optional_arg_with_all_args(): boolean {
    const result = optional_arg(withOptionalMethodInstanceClass, withoutOptionalMethodInstanceClass);

    return !!result.with && !!result.without;
}

function check_optional_arg_with_one_args(): boolean {
    const result = optional_arg(withOptionalMethodInstanceClass);

    return !!result.with;
}

// NOTE (issues 17772) fix spread operator and 
// uncomment function in check_spread_operator_arg_with_all_args

// function check_spread_operator_arg_with_all_args(): boolean {
//     const arr: (WithoutOptionalMethodClass | withoutOptionalMethodInstanceClass)[] = [withOptionalMethodInstanceClass, withoutOptionalMethodInstanceClass];
//     const result = optional_arg_array(...arr);
//     return !!result.with && !!result.without;
// }

// function check_spread_operator_arg_with_one_args(): boolean {
//     const arr: (WithoutOptionalMethodClass | WithoutOptionalMethodClass)[] = [withOptionalMethodInstanceClass];
//     const result = optional_arg_array(...arr);
//     return !!result.with;
// }

function check_without_spread_operator_arg_with_all_args(): boolean {
    const result = optional_arg_array(withOptionalMethodInstanceClass, withoutOptionalMethodInstanceClass);
    return !!result.with && !!result.without;
}

function check_without_spread_operator_arg_with_one_args(): boolean {
    const result = optional_arg_array(withOptionalMethodInstanceClass);
    return !!result.with;
}
