/*
* Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

import { Foo, fooInstance, fooFunc, testObject, testStr, testNumber, testStr50, testStr100,
    testStr500, testStr1000, testStr5000, testStr10000, testStr50000 } from 'testDecl';

const ITERATIONS = 1000;

function testGetObjectPropertis(): boolean {
    let f1 = fooInstance;
    for (let i = 0; i < ITERATIONS; i++) {
        let value = f1.objectProperty;
        if (value === undefined) {
            return false;
        }
    }
    return true;
}

function testSetObjectPropertis(): boolean {
    for (let i = 0; i < ITERATIONS; i++) {
        fooInstance.objectProperty = testObject;
    }
    return true;
}

function testGetStringPropertis(): boolean {
    let f1 = fooInstance;
    for (let i = 0; i < ITERATIONS; i++) {
        let value = f1.stringProperty;
        if (value !== testStr) {
            return false;
        }
    }
    return true;
}

function testSetStringPropertis(): boolean {
    let f1 = fooInstance;
    for (let i = 0; i < ITERATIONS; i++) {
        f1.stringProperty = testStr;
    }
    return true;
}

function testGetStringPropertis50(): boolean {
    let f1 = fooInstance;
    for (let i = 0; i < ITERATIONS; i++) {
        let value = f1.string50;
        if (value !== testStr50) {
            return false;
        }
    }
    return true;
}

function testSetStringPropertis50(): boolean {
    let f1 = fooInstance;
    for (let i = 0; i < ITERATIONS; i++) {
        f1.string50 = testStr50;
    }
    return true;
}

function testGetStringPropertis100(): boolean {
    let f1 = fooInstance;
    for (let i = 0; i < ITERATIONS; i++) {
        let value = f1.string100;
        if (value !== testStr100) {
            return false;
        }
    }
    return true;
}

function testSetStringPropertis100(): boolean {
    let f1 = fooInstance;
    for (let i = 0; i < ITERATIONS; i++) {
        f1.string100 = testStr100;
    }
    return true;
}

function testGetStringPropertis500(): boolean {
    let f1 = fooInstance;
    for (let i = 0; i < ITERATIONS; i++) {
        let value = f1.string500;
        if (value !== testStr500) {
            return false;
        }
    }
    return true;
}

function testSetStringPropertis500(): boolean {
    let f1 = fooInstance;
    for (let i = 0; i < ITERATIONS; i++) {
        f1.string500 = testStr500;
    }
    return true;
}

function testGetStringPropertis1000(): boolean {
    let f1 = fooInstance;
    for (let i = 0; i < ITERATIONS; i++) {
        let value = f1.string1000;
        if (value !== testStr1000) {
            return false;
        }
    }
    return true;
}

function testSetStringPropertis1000(): boolean {
    let f1 = fooInstance;
    for (let i = 0; i < ITERATIONS; i++) {
        f1.string1000 = testStr1000;
    }
    return true;
}

function testGetStringPropertis5000(): boolean {
    let f1 = fooInstance;
    for (let i = 0; i < ITERATIONS; i++) {
        let value = f1.string5000;
        if (value !== testStr5000) {
            return false;
        }
    }
    return true;
}

function testSetStringPropertis5000(): boolean {
    let f1 = fooInstance;
    for (let i = 0; i < ITERATIONS; i++) {
        f1.string5000 = testStr5000;
    }
    return true;
}

function testGetStringPropertis10000(): boolean {
    let f1 = fooInstance;
    for (let i = 0; i < ITERATIONS; i++) {
        let value = f1.string10000;
        if (value !== testStr10000) {
            return false;
        }
    }
    return true;
}

function testSetStringPropertis10000(): boolean {
    let f1 = fooInstance;
    for (let i = 0; i < ITERATIONS; i++) {
        f1.string10000 = testStr10000;
    }
    return true;
}

function testGetStringPropertis50000(): boolean {
    let f1 = fooInstance;
    for (let i = 0; i < ITERATIONS; i++) {
        let value = f1.string50000;
        if (value !== testStr50000) {
            return false;
        }
    }
    return true;
}

function testSetStringPropertis50000(): boolean {
    let f1 = fooInstance;
    for (let i = 0; i < ITERATIONS; i++) {
        f1.string50000 = testStr50000;
    }
    return true;
}

function testGetNumberPropertis(): boolean {
    let f1 = fooInstance;
    for (let i = 0; i < ITERATIONS; i++) {
        let value = f1.numberProperty;
        if (value !== testNumber) {
            return false;
        }
    }
    return true;
}

function testSetNumberPropertis(): boolean {
    let f1 = fooInstance;
    for (let i = 0; i < ITERATIONS; i++) {
        f1.numberProperty = testNumber;
    }
    return true;
}


function testCallFooFunc(): boolean {
    let f1 = fooFunc;
    let f2 = fooInstance;
    for (let i = 0; i < ITERATIONS; i++) {
        f1(f2);
    }
    return true;
}
