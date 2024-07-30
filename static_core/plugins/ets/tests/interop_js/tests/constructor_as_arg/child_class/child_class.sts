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
    ts_int,
    MainClass,
    AnonymousClass,
    ChildClass,
    create_child_class_with_arg_from_ts,
    create_child_class_with_main_from_ts,
    childClassWithMainInstance,
    create_child_class_with_anonymous_from_ts,
    childClassWithAnonymousInstance,
    create_child_class_with_IIFE_from_ts,
    childClassWithIIFEInstance,
    IIFEClass,
    check_instance,
} from "main_js"

function check_inheritor_class_main(): boolean {
    const TSClass = new ChildClass(MainClass);

    return check_instance(ChildClass, TSClass) as boolean
        && TSClass._other_class._value as int == ts_int as int;
}

function check_inheritor_class_anonymous(): boolean {
    const TSClass = new ChildClass(AnonymousClass);

    return check_instance(ChildClass, TSClass) as boolean
        && TSClass._other_class._value as int == ts_int as int;
}

function check_inheritor_class_IIFE(): boolean {
    const TSClass = new ChildClass(IIFEClass);

    return check_instance(ChildClass, TSClass) as boolean
        && TSClass._other_class._value as int == ts_int as int;
}

function check_create_child_class_with_arg_main_from_ts(): boolean {
    const TSClass = create_child_class_with_arg_from_ts(MainClass);

    return check_instance(ChildClass, TSClass) as boolean
        && TSClass._other_class._value as int == ts_int as int;
}

function check_create_child_class_with_arg_anonymous_from_ts(): boolean {
    const TSClass = create_child_class_with_arg_from_ts(AnonymousClass);

    return check_instance(ChildClass, TSClass) as boolean
        && TSClass._other_class._value as int == ts_int as int;
}

function check_create_child_class_with_arg_IIFE_from_ts(): boolean {
    const TSClass = create_child_class_with_arg_from_ts(IIFEClass);

    return check_instance(ChildClass, TSClass) as boolean
        && TSClass._other_class._value as int == ts_int as int;
}

function check_create_child_class_with_main_from_ts(): boolean {
    const TSClass = create_child_class_with_main_from_ts();

    return check_instance(ChildClass, TSClass) as boolean
        && TSClass._other_class._value as int == ts_int as int;
}

function check_child_class_with_main_instance(): boolean {
    const TSClass = childClassWithMainInstance;

    return check_instance(ChildClass, TSClass) as boolean
        && TSClass._other_class._value as int == ts_int as int;
}

function check_create_child_class_with_anonymous_from_ts(): boolean {
    const TSClass = create_child_class_with_anonymous_from_ts();

    return check_instance(ChildClass, TSClass) as boolean
        && TSClass._other_class._value as int == ts_int as int;
}

function check_child_class_with_anonymous_instance(): boolean {
    const TSClass = childClassWithAnonymousInstance;

    return check_instance(ChildClass, TSClass) as boolean
        && TSClass._other_class._value as int == ts_int as int;
}

function check_create_child_class_with_IIFE_from_ts(): boolean {
    const TSClass = create_child_class_with_IIFE_from_ts();

    return check_instance(ChildClass, TSClass) as boolean
        && TSClass._other_class._value as int == ts_int as int;
}

function check_child_class_with_IIFE_instance(): boolean {
    const TSClass = childClassWithIIFEInstance;

    return check_instance(ChildClass, TSClass) as boolean
        && TSClass._other_class._value as int == ts_int as int;
}
