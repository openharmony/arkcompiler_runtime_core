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
} from 'esvaluewithstringkey_js';

function checkFieldAccessByDot(): int {
    let e: JSValue = new A();
    let res = e.Ekko;
    if (res === 'ekko') {
        return 0;
    } else {
        return 1;
    }
}

function checkEsvalueWithStringLiteral(): int {
    let e: JSValue = new A();
    let res = e['Ekko'];
    if (res === 'ekko') {
        return 0;
    } else {
        return 1;
    }
}

function checkEsvalueWithStringKey() : int {
    let e: JSValue = new A();
    let stringKey: string = 'Ekko';
    let res = e[stringKey];
    if (res === 'ekko') {
        return 0;
    } else {
        return 1;
    }
}

function foo(es: JSValue, str: string): string {
    return es[str];
}

function checkEsvalueWithStringFromFoo() : int {
    let es: JSValue = new A();
    let stringKey: string = 'Ekko';
    let res: string = foo(es, stringKey);

    if (res === 'ekko') {
        return 0;
    } else {
        return 1;
    }
}

function checkEsvalueWithExpression(): int {
    let es: JSValue = new A();
    let stringKey: string = 'Ekk';
    let res = es[stringKey + 'o'];

    if (res === 'ekko') {
        return 0;
    } else {
        return 1;
    }
}

function checkEsvalueWithExpressionStrLiterals(): int {
    let es: JSValue = new A();
    let res = es['Ekk' + 'o'];

    if (res === 'ekko') {
        return 0;
    } else {
        return 1;
    }
}

function checkEsvalueWithExpressionStrings(): int {
    let es: JSValue = new A();
    let stringKey1: string = 'Ekk';
    let stringKey2: string = 'o';
    let res = es[stringKey1 + stringKey2];

    if (res === 'ekko') {
        return 0;
    } else {
        return 1;
    }
}
