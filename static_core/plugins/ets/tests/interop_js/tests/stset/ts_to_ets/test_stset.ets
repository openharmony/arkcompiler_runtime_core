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

let module = ESValue.load('stset');

function testReturnSet(): boolean {
    let returnSet = module.getProperty('returnSet');
    let result = returnSet.invoke();
    if (result.instanceOf(Type.from<Set<string>>())) {
        let set: Set<string> = result.unwrap() as Set<string>;
        return set.size === 0;
    }
    return false;
}

function testConsumeSet(): boolean {
    let consumeSet = module.getProperty('consumeSet');
    let set: Set<string> = new Set<string>();
    return consumeSet.invoke(set).toBoolean();
}

function testCheckUniqueness(): boolean {
    let checkUniqueness = module.getProperty('checkUniqueness');
    let set: Set<string> = new Set<string>();
    return checkUniqueness.invoke(set).toBoolean();
}

function testCheckSizeIsProperty(): boolean {
    let checkSizeIsProperty = module.getProperty('checkSizeIsProperty');
    let set: Set<string> = new Set<string>();
    return checkSizeIsProperty.invoke(set).toBoolean();
}

function testCheckInsertionOrder(): boolean {
    let checkInsertionOrder = module.getProperty('checkInsertionOrder');
    let set: Set<string> = new Set<string>();
    set.add('first');
    set.add('second');
    set.add('third');
    return checkInsertionOrder.invoke(set).toBoolean();
}

function testCheckChaining(): boolean {
    let checkChaining = module.getProperty('checkChaining');
    let set: Set<string> = new Set<string>();
    set.add('x').add('y').add('z');
    return checkChaining.invoke(set).toBoolean();
}

function testCheckNoIndexAccess(): boolean {
    let checkNoIndexAccess = module.getProperty('checkNoIndexAccess');
    let set: Set<string> = new Set<string>();
    set.add('x');
    set.add('y');
    return checkNoIndexAccess.invoke(set).toBoolean();
}

function testCheckDeleteReturnsBoolean(): boolean {
    let checkDeleteReturnsBoolean = module.getProperty('checkDeleteReturnsBoolean');
    let set: Set<string> = new Set<string>();
    set.add('test');
    return checkDeleteReturnsBoolean.invoke(set).toBoolean();
}

function testCheckHasMethod(): boolean {
    let checkHasMethod = module.getProperty('checkHasMethod');
    let set: Set<string> = new Set<string>();
    set.add('a');
    set.add('b');
    return checkHasMethod.invoke(set).toBoolean();
}

function testCheckClearMethod(): boolean {
    let checkClearMethod = module.getProperty('checkClearMethod');
    let set: Set<string> = new Set<string>();
    set.add('a');
    set.add('b');
    set.add('c');
    checkClearMethod.invoke(set);
    return set.size === 0;
}

function testInstanceOfSet(): boolean {
    let dynamicSet = module.getProperty('createEmptySet');
    return !(dynamicSet instanceof Set);
}

export function testSetProto(m: Set<number>): void { }
