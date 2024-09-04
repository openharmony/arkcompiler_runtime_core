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
    ParentClass,
    create_class_with_arg_from_ts,
    create_main_class_from_ts,
    create_anonymous_class_from_ts,
    anonymousClassInstance,
    create_IIFE_class_from_ts,
    iifeClassInstance,
    mainClassInstance,
    IIFEClass,
    check_instance,
} from "main_js"

function check_main_class_as_arg(): boolean {
    const TSClass = new ParentClass(MainClass);

    return check_instance(ParentClass, TSClass) as boolean
        && TSClass._other_class._value as int == ts_int as int;
}

function check_anonymous_class_as_arg(): boolean {
    const TSClass = new ParentClass(AnonymousClass);

    return check_instance(ParentClass, TSClass) as boolean
        && TSClass._other_class._value as int == ts_int as int;
}

function check_IIFE_class_as_arg(): boolean {
    const TSClass = new ParentClass(IIFEClass);

    return check_instance(ParentClass, TSClass) as boolean
        && TSClass._other_class._value as int == ts_int as int;
}

function check_create_main_class_with_arg_from_ts(): boolean {
    const TSClass = create_class_with_arg_from_ts(MainClass);

    return check_instance(ParentClass, TSClass) as boolean
        && TSClass._other_class._value as int == ts_int as int;
}

function check_create_main_class_from_ts(): boolean {
    const TSClass = create_main_class_from_ts();

    return check_instance(ParentClass, TSClass) as boolean
        && TSClass._other_class._value as int == ts_int as int;
}

function check_main_class_instance(): boolean {
    const TSClass = mainClassInstance;

    return check_instance(ParentClass, TSClass) as boolean
        && TSClass._other_class._value as int == ts_int as int;
}

function check_create_anonymous_class_with_arg_from_ts(): boolean {
    const TSClass = create_class_with_arg_from_ts(AnonymousClass);

    return check_instance(ParentClass, TSClass) as boolean
        && TSClass._other_class._value as int == ts_int as int;
}

function check_create_anonymous_class_from_ts(): boolean {
    const TSClass = create_anonymous_class_from_ts();

    return check_instance(ParentClass, TSClass) as boolean
        && TSClass._other_class._value as int == ts_int as int;
}

function check_anonymous_class_instance(): boolean {
    const TSClass = anonymousClassInstance;

    return check_instance(ParentClass, TSClass) as boolean
        && TSClass._other_class._value as int == ts_int as int;
}

function check_create_iife_class_from_ts(): boolean {
    const TSClass = create_IIFE_class_from_ts();

    return check_instance(ParentClass, TSClass) as boolean
        && TSClass._other_class._value as int == ts_int as int;
}

function check_iife_class_instance(): boolean {
    const TSClass = iifeClassInstance;

    return check_instance(ParentClass, TSClass) as boolean
        && TSClass._other_class._value as int == ts_int as int;
}
