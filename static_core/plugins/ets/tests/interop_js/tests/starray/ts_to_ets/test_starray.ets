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

let module = ESValue.load('starray');

function testReturnArray(): boolean {
    let returnArray = module.getProperty('returnArray');
    let result = returnArray.invoke();
    if (result.instanceOf(Type.from<Array<string>>())) {
        let arr: string[] = result.unwrap() as string[];
        return arr.length === 0;
    }
    return false;
}

function testConsumeArray(): boolean {
    let consumeArray = module.getProperty('consumeArray');
    let arr: string[] = [];
    arr.push('1');
    arr.push('2');
    return consumeArray.invoke(arr).toBoolean();
}

function testInstanceOfArray(): boolean {
    let dynamicArray = module.getProperty('createEmptyArray');
    return !(dynamicArray instanceof Array);
}

export function testArrayProto(arr: Array<number>): void { }
