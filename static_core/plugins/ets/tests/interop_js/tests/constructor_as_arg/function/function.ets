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
    ParentClass,
    ChildClass,
    AnonymousClass,
    IIFEClass,
    create_class_function,
    create_class_arrow_function,
    check_instance,
} from "main_js"


function check_create_class_function_main(): boolean {
    const TSClass = create_class_function(MainClass, ts_int);

    return check_instance(MainClass, TSClass) as boolean
        && TSClass._value as int == ts_int as int;
}

function check_create_class_function_parent(): boolean {
    const TSClass = create_class_function(ParentClass, MainClass);

    return check_instance(ParentClass, TSClass) as boolean
        && TSClass._other_class._value as int == ts_int as int;
}

function check_create_class_function_child(): boolean {
    const TSClass = create_class_function(ChildClass, MainClass);

    return check_instance(ChildClass, TSClass) as boolean
        && TSClass._other_class._value as int == ts_int as int;
}

function check_create_class_function_anonymous(): boolean {
    const TSClass = create_class_function(AnonymousClass, ts_int);

    return check_instance(AnonymousClass, TSClass) as boolean
        && TSClass._value as int == ts_int as int;
}

function check_create_class_function_IIFE(): boolean {
    const TSClass = create_class_function(IIFEClass, ts_int);

    return check_instance(IIFEClass, TSClass) as boolean
        && TSClass._value as int == ts_int as int;
}

function check_create_class_arrow_function_main(): boolean {
    const TSClass = create_class_arrow_function(MainClass, ts_int);

    return check_instance(MainClass, TSClass) as boolean
        && TSClass._value as int == ts_int as int;
}

function check_create_class_arrow_function_parent(): boolean {
    const TSClass = create_class_arrow_function(ParentClass, MainClass);

    return check_instance(ParentClass, TSClass) as boolean
        && TSClass._other_class._value as int == ts_int as int;
}

function check_create_class_arrow_function_child(): boolean {
    const TSClass = create_class_arrow_function(ChildClass, MainClass);

    return check_instance(ChildClass, TSClass) as boolean
        && TSClass._other_class._value as int == ts_int as int;
}

function check_create_class_arrow_function_anonymous(): boolean {
    const TSClass = create_class_arrow_function(AnonymousClass, ts_int);

    return check_instance(AnonymousClass, TSClass) as boolean
        && TSClass._value as int == ts_int as int;
}

function check_create_class_arrow_function_IIFE(): boolean {
    const TSClass = create_class_arrow_function(IIFEClass, ts_int);

    return check_instance(IIFEClass, TSClass) as boolean
        && TSClass._value as int == ts_int as int;
}
