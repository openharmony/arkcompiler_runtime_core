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
    ts_number,
    PublicGetterClass,
    create_public_getter_class_from_ts,
    publicGetterInstanceClass,
    UnionTypeClass,
    create_union_type_getter_class_from_ts,
    unionTypeGetterInstanceClassInt,
    unionTypeGetterInstanceClassString,
    LiteralClass,
    create_literal_type_getter_class_from_ts,
    literalTypeGetterInstanceClassInt,
    literalTypeGetterInstanceClassString,
    TupleType,
    TupleTypeClass,
    create_tuple_type_getter_class_from_ts,
    tupleTypeGetterInstanceClass,
    AnyTypeClass,
    create_any_type_getter_class_from_ts,
    anyTypeGetterInstanceClass,
    anyTypeExplicitGetterInstanceClass,
    SubsetByRef,
    create_subset_by_ref_getter_class_from_ts,
    subsetByRefInstanceClass,
    SubsetByValueClass,
    create_subset_by_value_getter_class_from_ts,
    subsetByValueInstanceClass,
} from '/plugins/ets/tests/interop_js/tests/getter/ts_to_ets/getter.js';


const tuple_arr: TupleType = [ts_number, ts_string];
const ets_number = 1;
const ets_string = 'string';
const ets_arr: int[] = [ets_number];
const ets_bool = true;
const ets_obj: Object = {};
const ets_union: int | string = ets_number;

function check_getter_public_class(): boolean {
    const GClass = new PublicGetterClass();

    return GClass.value as string == ts_string as string;
}

function check_create_public_getter_class_from_ts(): boolean {
    const GClass = create_public_getter_class_from_ts();

    return ts_string as string == GClass.value as string;
}

function check_public_getter_instance_class_from_ts(): boolean {
    return ts_string as string == publicGetterInstanceClass.value as string;
}

function check_union_type_getter_class_int(): boolean {
    const GClass = new UnionTypeClass(ts_number);

    return GClass.value as int == ts_number as int;
}

function check_create_union_type_getter_class_from_ts_int(): boolean {
    const GClass = create_union_type_getter_class_from_ts(ts_number);

    return GClass.value as int == ts_number as int;
}

function check_instance_union_type_getter_class_from_ts_int(): boolean {
    return ts_number as int == unionTypeGetterInstanceClassInt.value as int;
}

function check_union_type_getter_class_string(): boolean {
    const GClass = new UnionTypeClass(ts_string);

    return GClass.value as string == ts_string as string;
}

function check_create_union_type_getter_class_from_ts_string(): boolean {
    const GClass = create_union_type_getter_class_from_ts(ts_string);

    return GClass.value as string == ts_string as string;
}

function check_instance_union_type_getter_class_from_ts_string(): boolean {
    return ts_string as string == unionTypeGetterInstanceClassString.value as string;
}

function check_literal_type_getter_class_int(): boolean {
    const GClass = new LiteralClass(ts_number);

    return GClass.value as int == ts_number as int;
}

function check_literal_type_getter_class_string(): boolean {
    const GClass = new LiteralClass(ts_string);

    return GClass.value as string == ts_string as string;
}

function check_create_literal_type_getter_class_from_ts_int(): boolean {
    const GClass = create_literal_type_getter_class_from_ts(ts_number);

    return GClass.value as int == ts_number as int;
}

function check_create_literal_type_getter_class_from_ts_string(): boolean {
    const GClass = create_literal_type_getter_class_from_ts(ts_string);

    return GClass.value as string == ts_string as string;
}

function check_instance_literal_type_getter_class_from_ts_int(): boolean {
    return ts_number as int == literalTypeGetterInstanceClassInt.value as int;
}

function check_instance_literal_type_getter_class_from_ts_string(): boolean {
    return ts_string as string == literalTypeGetterInstanceClassString.value as string;
}

function check_tuple_type_getter_class(): boolean {
    const GClass = new TupleTypeClass(tuple_arr);

    return GClass.value as Array<TupleType> instanceof Array
        && GClass.value[0] as int == tuple_arr[0] as int
        && GClass.value[1] as string == tuple_arr[1] as string;
}

function check_create_tuple_type_getter_class_from_ts(): boolean {
    const GClass = create_tuple_type_getter_class_from_ts(tuple_arr);

    return GClass.value as Array<TupleType> instanceof Array
        && GClass.value[0] as int == tuple_arr[0] as int
        && GClass.value[1] as string == tuple_arr[1] as string;
}

function check_instance_tuple_type_getter_class_from_ts(): boolean {
    const GClass = tupleTypeGetterInstanceClass;

    return GClass.value as Array<TupleType> instanceof Array
        && GClass.value[0] as int == tuple_arr[0] as int
        && GClass.value[1] as string == tuple_arr[1] as string;
}

function check_any_type_getter_class_int(): boolean {
    const GClass = new AnyTypeClass();

    GClass._value = ets_number;

    return GClass.value as int == ets_number as int
}

function check_any_type_getter_class_string(): boolean {
    const GClass = new AnyTypeClass();

    GClass._value = ets_string;

    return GClass.value as string == ets_string as string;
}

function check_any_type_getter_class_bool(): boolean {
    const GClass = new AnyTypeClass();

    GClass._value = ets_bool;

    return GClass.value as boolean == ets_bool as boolean;
}

function check_any_type_getter_class_arr(): boolean {
    const GClass = new AnyTypeClass();

    GClass._value = ets_arr;

    return GClass.value as Array<int> instanceof Array;
}

function check_any_type_getter_class_obj(): boolean {
    const GClass = new AnyTypeClass();

    GClass._value = ets_obj;

    return typeof GClass.value as Object == 'object';
}

function check_any_type_getter_class_tuple(): boolean {
    const GClass = new AnyTypeClass();

    GClass._value = tuple_arr;

    return GClass.value as Array<TupleType> instanceof Array
        && GClass.value[0] as int == tuple_arr[0] as int
        && GClass.value[1] as string == tuple_arr[1] as string;
}

function check_any_type_getter_class_union(): boolean {
    const GClass = new AnyTypeClass();

    GClass._value = ets_union;

    return GClass.value as int == ets_union as int;
}

function check_create_any_type_getter_class_from_ts_int(): boolean {
    const GClass = create_any_type_getter_class_from_ts();

    GClass._value = ets_number;

    return GClass.value as int == ets_number as int
}

function check_create_any_type_getter_class_from_ts_string(): boolean {
    const GClass = create_any_type_getter_class_from_ts();

    GClass._value = ets_string;

    return GClass.value as string == ets_string as string;
}

function check_create_any_type_getter_class_from_ts_bool(): boolean {
    const GClass = create_any_type_getter_class_from_ts();

    GClass._value = ets_bool;

    return GClass.value as boolean == ets_bool as boolean;
}

function check_create_any_type_getter_class_from_ts_arr(): boolean {
    const GClass = create_any_type_getter_class_from_ts();

    GClass._value = ets_arr;

    return GClass.value as Array<int> instanceof Array;
}

function check_create_any_type_getter_class_from_ts_obj(): boolean {
    const GClass = create_any_type_getter_class_from_ts();

    GClass._value = ets_obj;

    return typeof GClass.value as Object == 'object';
}

function check_create_any_type_getter_class_from_ts_tuple(): boolean {
    const GClass = create_any_type_getter_class_from_ts();

    GClass._value = tuple_arr;

    return GClass.value as Array<TupleType> instanceof Array
        && GClass.value[0] as int == tuple_arr[0] as int
        && GClass.value[1] as string == tuple_arr[1] as string;
}

function check_create_any_type_getter_class_from_ts_union(): boolean {
    const GClass = create_any_type_getter_class_from_ts();

    GClass._value = ets_union;

    return GClass.value as int == ets_union as int;
}

function check_instance_any_type_getter_class_from_ts_int(): boolean {
    const GClass = anyTypeGetterInstanceClass;

    GClass._value = ets_number;

    return GClass.value as int == ets_number as int
}

function check_instance_any_type_getter_class_from_ts_string(): boolean {
    const GClass = anyTypeGetterInstanceClass;

    GClass._value = ets_string;

    return GClass.value as string == ets_string as string;
}

function check_instance_any_type_getter_class_from_ts_bool(): boolean {
    const GClass = anyTypeGetterInstanceClass;

    GClass._value = ets_bool;

    return GClass.value as boolean == ets_bool as boolean;
}

function check_instance_any_type_getter_class_from_ts_arr(): boolean {
    const GClass = anyTypeGetterInstanceClass;

    GClass._value = ets_arr;

    return GClass.value as Array<int> instanceof Array;
}

function check_instance_any_type_getter_class_from_ts_obj(): boolean {
    const GClass = anyTypeGetterInstanceClass;

    GClass._value = ets_obj;

    return typeof GClass.value as Object == 'object';
}

function check_instance_any_type_getter_class_from_ts_tuple(): boolean {
    const GClass = anyTypeGetterInstanceClass;

    GClass._value = tuple_arr;

    return GClass.value as Array<TupleType> instanceof Array
        && GClass.value[0] as int == tuple_arr[0] as int
        && GClass.value[1] as string == tuple_arr[1] as string;
}

function check_instance_any_type_getter_class_from_ts_union(): boolean {
    const GClass = anyTypeGetterInstanceClass;

    GClass._value = ets_union;

    return GClass.value as int == ets_union as int;
}

function check_instance_any_explicit_type_getter_class_from_ts_explicit(): boolean {
    const GClass = anyTypeExplicitGetterInstanceClass;

    GClass._value = ets_string;

    return GClass.value as string == ets_string as string;
}

function check_getter_subset_by_ref_class(): boolean {
    const GClass = new SubsetByRef();

    return GClass.value as string == ts_string as string;
}

function check_create_subset_by_ref_getter_class_from_ts(): boolean {
    const GClass = create_subset_by_ref_getter_class_from_ts();

    return ts_string as string == GClass.value as string;
}

function check_instance_subset_by_ref_getter_class_from_ts(): boolean {
    const GClass = subsetByRefInstanceClass;

    return GClass.value as string == ets_string as string;
}

function check_getter_subset_by_value_class(): boolean {
    const GClass = new SubsetByValueClass(new PublicGetterClass().value);

    return GClass.value as string == ts_string as string;
}

function check_create_subset_by_value_getter_class_from_ts(): boolean {
    const GClass = create_subset_by_value_getter_class_from_ts();

    return ts_string as string == GClass.value as string;
}

function check_instance_subset_by_value_getter_class_from_ts(): boolean {
    const GClass = subsetByValueInstanceClass;

    return GClass.value as string == ets_string as string;
}