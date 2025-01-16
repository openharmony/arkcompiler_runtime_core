/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
    A,
} from 'esobjectwithstringkey_js';


function check_field_access_by_dot(): int {
    let e: JSValue = new A();
    let res = e.Ekko;
    if (res == 'ekko') {
        return 0;
    }
    else {
        return 1;
    }
}


function check_esobject_with_string_literal(): int {
    let e: JSValue = new A();
    let res = e['Ekko'];
    if (res == 'ekko') {
        return 0;
    }
    else {
        return 1;
    }
}

function check_esobject_with_string_key() : int {
    let e:  JSValue = new A();
    let string_key: string = 'Ekko';
    let res = e[string_key];
    if (res == 'ekko') {
        return 0;
    }
    else {
        return 1;
    }
}


function foo (es: JSValue, str: string): string {
    return es[str];
}

function check_esobject_with_string_from_foo() : int {
    let es:  JSValue = new A();
    let string_key: string = 'Ekko';
    let res: string = foo(es, string_key);

    if (res == 'ekko') {
        return 0;
    }
    else {
        return 1;
    }
}

function check_esobject_with_expression(): int {
    let es:  JSValue = new A();
    let string_key: string = 'Ekk';
    let res = es[string_key + 'o'];

    if (res == 'ekko') {
        return 0;
    }
    else {
        return 1;
    }
}

function check_esobject_with_expression_str_literals(): int {
    let es:  JSValue = new A();
    let res = es['Ekk' + 'o'];

    if (res == 'ekko') {
        return 0;
    }
    else {
        return 1;
    }
}

function check_esobject_with_expression_strings(): int {
    let es:  JSValue = new A();
    let string_key_1: string = 'Ekk';
    let string_key_2: string = 'o';
    let res = es[string_key_1 + string_key_2];

    if (res == 'ekko') {
        return 0;
    }
    else {
        return 1;
    }
}