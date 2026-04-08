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

let module = ESValue.load('stmap');

function testReturnMap(): boolean {
    let returnMap = module.getProperty('returnMap');
    let result = returnMap.invoke();
    if (result.instanceOf(Type.from<Map<string, double>>())) {
        let map: Map<string, double> = result.unwrap() as Map<string, double>;
        return map.size === 0;
    }
    return false;
}

function testConsumeMap(): boolean {
    let consumeMap = module.getProperty('consumeMap');
    let map: Map<string, double> = new Map<string, double>();
    return consumeMap.invoke(map).toBoolean();
}

function testCheckDuplicateKeyOverwrites(): boolean {
    let checkDuplicateKeyOverwrites = module.getProperty('checkDuplicateKeyOverwrites');
    let map: Map<string, double> = new Map<string, double>();
    return checkDuplicateKeyOverwrites.invoke(map).toBoolean();
}

function testCheckSizeIsProperty(): boolean {
    let checkSizeIsProperty = module.getProperty('checkSizeIsProperty');
    let map: Map<string, double> = new Map<string, double>();
    return checkSizeIsProperty.invoke(map).toBoolean();
}

function testCheckInsertionOrder(): boolean {
    let checkInsertionOrder = module.getProperty('checkInsertionOrder');
    let map: Map<string, double> = new Map<string, double>();
    map.set('first', 1);
    map.set('second', 2);
    map.set('third', 3);
    return checkInsertionOrder.invoke(map).toBoolean();
}

function testCheckChaining(): boolean {
    let checkChaining = module.getProperty('checkChaining');
    let map: Map<string, double> = new Map<string, double>();
    map.set('x', 1).set('y', 2).set('z', 3);
    return checkChaining.invoke(map).toBoolean();
}

function testCheckGetReturnsUndefined(): boolean {
    let checkGetReturnsUndefined = module.getProperty('checkGetReturnsUndefined');
    let map: Map<string, double> = new Map<string, double>();
    return checkGetReturnsUndefined.invoke(map).toBoolean();
}

function testCheckDeleteReturnsBoolean(): boolean {
    let checkDeleteReturnsBoolean = module.getProperty('checkDeleteReturnsBoolean');
    let map: Map<string, double> = new Map<string, double>();
    map.set('test', 1);
    return checkDeleteReturnsBoolean.invoke(map).toBoolean();
}

function testCheckHasMethod(): boolean {
    let checkHasMethod = module.getProperty('checkHasMethod');
    let map: Map<string, double> = new Map<string, double>();
    map.set('a', 1);
    map.set('b', 2);
    return checkHasMethod.invoke(map).toBoolean();
}

function testCheckClearMethod(): boolean {
    let checkClearMethod = module.getProperty('checkClearMethod');
    let map: Map<string, double> = new Map<string, double>();
    map.set('a', 1);
    map.set('b', 2);
    map.set('c', 3);
    checkClearMethod.invoke(map);
    return map.size === 0;
}

function testCheckForEachMethod(): boolean {
    let checkForEachMethod = module.getProperty('checkForEachMethod');
    let map: Map<string, double> = new Map<string, double>();
    return checkForEachMethod.invoke(map).toBoolean();
}

function testCheckEntriesMethod(): boolean {
    let checkEntriesMethod = module.getProperty('checkEntriesMethod');
    let map: Map<string, double> = new Map<string, double>();
    return checkEntriesMethod.invoke(map).toBoolean();
}

function testCheckKeysMethod(): boolean {
    let checkKeysMethod = module.getProperty('checkKeysMethod');
    let map: Map<string, double> = new Map<string, double>();
    return checkKeysMethod.invoke(map).toBoolean();
}

function testCheckValuesMethod(): boolean {
    let checkValuesMethod = module.getProperty('checkValuesMethod');
    let map: Map<string, double> = new Map<string, double>();
    return checkValuesMethod.invoke(map).toBoolean();
}

function testInstanceOfMap(): boolean {
    let dynamicMap = module.getProperty('createEmptyMap');
    return !(dynamicMap instanceof Map);
}

export function testMapProto(m: Map<number, number>): void { }
