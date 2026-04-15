/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

let ITERATIONS = 1000;
let module = ESValue.load('ESValueWorkload');
let fooInstance = module.getProperty('fooInstance');

function testGetPropertyString50(): boolean {
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        let val = fooInstance.getProperty('string50');
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload GetPropertyString50, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testGetPropertyString100(): boolean {
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        let val = fooInstance.getProperty('string100');
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload GetPropertyString100, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testGetPropertyString500(): boolean {
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        let val = fooInstance.getProperty('string500');
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload GetPropertyString500, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testGetPropertyString1000(): boolean {
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        let val = fooInstance.getProperty('string1000');
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload GetPropertyString1000, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testGetPropertyString5000(): boolean {
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        let val = fooInstance.getProperty('string5000');
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload GetPropertyString5000, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testGetPropertyString10000(): boolean {
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        let val = fooInstance.getProperty('string10000');
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload GetPropertyString10000, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testGetPropertyString50000(): boolean {
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        let val = fooInstance.getProperty('string50000');
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload GetPropertyString50000, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testGetPropertyBoolean(): boolean {
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        let val = fooInstance.getProperty('booleanField');
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload GetPropertyBoolean, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testGetPropertyNumber(): boolean {
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        let val = fooInstance.getProperty('numberField');
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload GetPropertyNumber, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testGetPropertyObject(): boolean {
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        let val = fooInstance.getProperty('objectField');
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload GetPropertyObject, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testSetPropertyString50(): boolean {
    let string50 = new Array<string>(50).fill('a').join('');
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        fooInstance.setProperty('string50', string50);
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload SetPropertyString50, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testSetPropertyString100(): boolean {
    let string100 = new Array<string>(100).fill('a').join('');
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        fooInstance.setProperty('string100', string100);
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload SetPropertyString100, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testSetPropertyString500(): boolean {
    let string500 = new Array<string>(500).fill('a').join('');
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        fooInstance.setProperty('string500', string500);
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload SetPropertyString500, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testSetPropertyString1000(): boolean {
    let string1000 = new Array<string>(1000).fill('a').join('');
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        fooInstance.setProperty('string1000', string1000);
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload SetPropertyString1000, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testSetPropertyString5000(): boolean {
    let string5000 = new Array<string>(5000).fill('a').join('');
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        fooInstance.setProperty('string5000', string5000);
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload SetPropertyString5000, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testSetPropertyString10000(): boolean {
    let string10000 = new Array<string>(10000).fill('a').join('');
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        fooInstance.setProperty('string10000', string10000);
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload SetPropertyString10000, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testSetPropertyString50000(): boolean {
    let string50000 = new Array<string>(50000).fill('a').join('');
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        fooInstance.setProperty('string50000', string50000);
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload SetPropertyString50000, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testSetPropertyBoolean(): boolean {
    let boolValue = true;
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        fooInstance.setProperty('booleanField', boolValue);
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload SetPropertyBoolean, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testSetPropertyNumber(): boolean {
    let numValue = 42.5;
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        fooInstance.setProperty('numberField', numValue);
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload SetPropertyNumber, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testSetPropertyObject(): boolean {
    let objValue = new Object();
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        fooInstance.setProperty('objectField', objValue);
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload SetPropertyObject, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

// invoke tests
function testInvokeNoArgs(): boolean {
    let func = module.getProperty('functionNoArgs');
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        let result = func.invoke();
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload InvokeNoArgs, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testInvokeOneArgNumber(): boolean {
    let func = module.getProperty('functionOneArgNumber');
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        let result = func.invoke(42.5);
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload InvokeOneArgNumber, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testInvokeOneArgString(): boolean {
    let func = module.getProperty('functionOneArgString');
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        let result = func.invoke('test');
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload InvokeOneArgString, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testInvokeOneArgObject(): boolean {
    let func = module.getProperty('functionOneArgObject');
    let obj = new Object();
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        let result = func.invoke(obj);
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload InvokeOneArgObject, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testInvokeFiveArgsNumber(): boolean {
    let func = module.getProperty('functionFiveArgsNumber');
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        let result = func.invoke(1, 2, 3, 4, 5);
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload InvokeFiveArgsNumber, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testInvokeFiveArgsString(): boolean {
    let func = module.getProperty('functionFiveArgsString');
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        let result = func.invoke('a', 'b', 'c', 'd', 'e');
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload InvokeFiveArgsString, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testInvokeFiveArgsObject(): boolean {
    let func = module.getProperty('functionFiveArgsObject');
    let obj1 = new Object();
    let obj2 = new Object();
    let obj3 = new Object();
    let obj4 = new Object();
    let obj5 = new Object();
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        let result = func.invoke(obj1, obj2, obj3, obj4, obj5);
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload InvokeFiveArgsObject, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

// invokeMethod tests
function testInvokeMethodNoArgs(): boolean {
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        let result = fooInstance.invokeMethod('methodNoArgs');
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload InvokeMethodNoArgs, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testInvokeMethodOneArgNumber(): boolean {
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        let result = fooInstance.invokeMethod('methodOneArgNumber', 42.5);
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload InvokeMethodOneArgNumber, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testInvokeMethodOneArgString(): boolean {
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        let result = fooInstance.invokeMethod('methodOneArgString', 'test');
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload InvokeMethodOneArgString, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testInvokeMethodOneArgObject(): boolean {
    let obj = new Object();
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        let result = fooInstance.invokeMethod('methodOneArgObject', obj);
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload InvokeMethodOneArgObject, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testInvokeMethodFiveArgsNumber(): boolean {
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        let result = fooInstance.invokeMethod('methodFiveArgsNumber', 1, 2, 3, 4, 5);
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload InvokeMethodFiveArgsNumber, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testInvokeMethodFiveArgsString(): boolean {
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        let result = fooInstance.invokeMethod('methodFiveArgsString', 'a', 'b', 'c', 'd', 'e');
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload InvokeMethodFiveArgsString, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}

function testInvokeMethodFiveArgsObject(): boolean {
    let obj1 = new Object();
    let obj2 = new Object();
    let obj3 = new Object();
    let obj4 = new Object();
    let obj5 = new Object();
    let startTime = Chrono.nanoNow();
    for (let i = 0; i < ITERATIONS; i++) {
        let result = fooInstance.invokeMethod('methodFiveArgsObject', obj1, obj2, obj3, obj4, obj5);
    }
    let endTime = Chrono.nanoNow();
    console.log(`esvalue_workload InvokeMethodFiveArgsObject, ${(endTime - startTime) / ITERATIONS}`);
    return true;
}
