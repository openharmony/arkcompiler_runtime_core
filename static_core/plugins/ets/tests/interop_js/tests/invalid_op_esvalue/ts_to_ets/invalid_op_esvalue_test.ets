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

let module = ESValue.load('../../plugins/ets/tests/interop_js/tests/invalid_op_esvalue/ts_to_ets/test_invalid_op_esvalue');

function checkUndefinedGetProperty(): boolean {
    let res: boolean = false;
    try {
        ESValue.Undefined.getProperty('name');
    } catch (e: Error) {
        res = e.message === 'Need object';
    }
    return res;
}

function checkUndefinedHasProperty(): boolean {
    let res: boolean = false;
    try {
        ESValue.Undefined.hasProperty('test');
    } catch (e: Error) {
        res = e.message === 'Need object';
    }
    return res;
}

function checkUndefinedToNumber(): boolean {
    let res: boolean = false;
    try {
        ESValue.Undefined.toNumber();
    } catch (e: Error) {
        res = e.message === 'Number expected';
    }
    return res;
}

function checkUndefinedToString(): boolean {
    let res: boolean = false;
    try {
        ESValue.Undefined.toString();
    } catch (e: Error) {
        res = e.message === 'String expected';
    }
    return res;
}

function checkUndefinedSetIndex(): boolean {
    let res: boolean = false;
    try {
        ESValue.Undefined.setProperty(1, 1);
    } catch (e: Error) {
        res = e.message === 'Need object';
    }
    return res;
}

function checkUndefinedInvokeMethod(): boolean {
    let res: boolean = false;
    try {
        ESValue.Undefined.invokeMethod('test');
    } catch (e: Error) {
        res = e.message === 'Need object';
    }
    return res;
}

function checkUndefinedInstantiate(): boolean {
    let res: boolean = false;
    try {
        ESValue.Undefined.instantiate();
    } catch (e: Error) {
        res = e.message === 'Need function';
    }
    return res;
}

function checkNullSetProperty(): boolean {
    let res: boolean = false;
    try {
        ESValue.Null.setProperty('name', 'value');
    } catch (e: Error) {
        res = e.message === 'Need object';
    }
    return res;
}

function checkNullHasOwnProperty(): boolean {
    let res: boolean = false;
    try {
        ESValue.Null.hasOwnProperty('name');
    } catch (e: Error) {
        res = e.message === 'Need object';
    }
    return res;
}

function checkNullToString(): boolean {
    let res: boolean = false;
    try {
        ESValue.Null.toString();
    } catch (e: Error) {
        res = e.message === 'String expected';
    }
    return res;
}

function checkNullGetIndex(): boolean {
    let res: boolean = false;
    try {
        ESValue.Null.getProperty(1);
    } catch (e: Error) {
        res = e.message === 'Need object';
    }
    return res;
}

function checkNullInvoke(): boolean {
    let res: boolean = false;
    try {
        ESValue.Null.invoke();
    } catch (e: Error) {
        res = e.message === 'Need function';
    }
    return res;
}

function checkIntGetProperty(): boolean {
    let res: boolean = false;
    try {
	    ESValue.wrapInt(123).getProperty('123');
    } catch (e: Error) {
        res = e.message === 'Need object';
    }
    return res;
}

function checkIntToString(): boolean {
    let res: boolean = false;
    try {
	    ESValue.wrapInt(123).toString();
    } catch (e: Error) {
        res = e.message === 'String expected';
    }
    return res;
}

function checkIntInvoke(): boolean {
    let res: boolean = false;
    try {
	    ESValue.wrapInt(123).invoke();
    } catch (e: Error) {
        res = e.message === 'Need function';
    }
    return res;
}

function checkIntHasProperty(): boolean {
    let res: boolean = false;
    try {
        ESValue.wrapInt(123).hasProperty('123');
    } catch (e: Error) {
        res = e.message === 'Need object';
    }
    return res;
}

function checkBooleanInvokeMethod(): boolean {
    let res: boolean = false;
    try {
	    ESValue.wrapBoolean(false).invoke('test');
    } catch (e: Error) {
        res = e.message === 'Need function';
    }
    return res;
}

function checkBigintInstantiate(): boolean {
    let res: boolean = false;
    try {
	    ESValue.wrapBigInt(123n).instantiate();
    } catch (e: Error) {
        res = e.message === 'Need function';
    }
    return res;
}

function checkStringSetProperty(): boolean {
    let res: boolean = false;
    try {
        ESValue.wrapString('123').setProperty('ID', ESValue.wrapInt(456));
    } catch (e: Error) {
        res = e.message === 'Need object';
    }
    return res;
}

function checkIntSetIndex(): boolean {
    let res: boolean = false;
    try {
        ESValue.wrapInt(123).setProperty(1, ESValue.wrapInt(456));
    } catch (e: Error) {
        res = e.message === 'Need object';
    }
    return res;
}

function checkTsGetProperty(): boolean {
    let res: boolean = false;
    try {
        module.getProperty('Base').getProperty('base').getProperty('base1');
    } catch (e: Error) {
        res = e.message === 'Need object';
    }
    return res;
}

function checkTsInstantiate(): boolean {
    let res: boolean = false;
    try {
        module.getProperty('base').instantiate();
    } catch (e: Error) {
        res = e.message === 'Need function';
    }
    return res;
}

function checkTsInvoke(): boolean {
    let res: boolean = false;
    try {
        module.getProperty('base').invoke();
    } catch (e: Error) {
        res = e.message === 'Need function';
    }
    return res;
}

function checkTsGetIndex(): boolean {
    let res: boolean = false;
    try {
        module.getProperty('base').getProperty(1).getProperty(1);
    } catch (e: Error) {
        res = e.message === 'Need object';
    }
    return res;
}

function checkTsHasProperty(): boolean {
    let res: boolean = false;
    try {
        module.getProperty('base').getProperty(1).hasProperty(ESValue.wrap(111));
    } catch (e: Error) {
        res = e.message === 'Need object';
    }
    return res;
}

function checkTsToBoolean(): boolean {
    let res: boolean = false;
    try {
        module.getProperty('base').toBoolean();
    } catch (e: Error) {
        res = e.message === 'Boolean expected';
    }
    return res;
}


function checkTsToPromise(): boolean {
    let res: boolean = false;
    try {
        module.getProperty('base').toPromise();
    } catch (e: Error) {
        res = e.message === 'Promise expected';
    }
    return res;
}

function checkTsToString(): boolean {
    let res: boolean = false;
    try {
        module.getProperty('base').getProperty('baseVal').toString();
    } catch (e: Error) {
        res = e.message === 'String expected';
    }
    return res;
}