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

let constArray: string[] = []; 

export function getConstArray(): Array<string> {
    return constArray;
}

export function customLengthArray(length: int): Array<string> {
    return new Array<string>(length);
}

export function testArrayValByIndex(arr: Array<string>, idx: int, val: string): boolean {
    return arr[idx] === val;
}

export function setArrayValByIndex(arr: Array<string>, idx: int, val: string) {
    arr[idx] = val;
}

export class User {
    private name_: string;
    private age_: int;

    constructor(name: string, age: int) {
        this.name_ = name;
        this.age_ = age;
    }

    get name(): string {
        return this.name_;
    }
    get age(): int {
        return this.age_;
    }
}

export function returnUserArray(): Array<User> {
    let arr: User[] = []; 
    arr.push(new User('user3', 11));
    arr.push(new User('user4', 22));
    return arr;
}

export function testUserArrayName(arr: Array<User>, idx: int, val: string): boolean {
    return arr[idx].name === val;
}

export function testUserArrayAge(arr: Array<User>, idx: int, val: int): boolean {
    return arr[idx].age === val;
}

export function esFuncArrayTest(func: (val: Array<string>) => void, arr: Array<string>) {
    func(arr);
}
