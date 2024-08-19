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

export function fnWithAnyParams(arr?: any[]): object {
    return arr ? arr[0] : 'Argument not found';
}


type City = 'Moscow' | 'London' | 'Paris';

export function fnWithLiteralParam(arr?: City[]): object {
    return arr ? arr[0] : 'Argument not found';
}

export function fnWithExtraSetParam(arr?: unknown): object {
    return arr ? arr[0] : 'Argument not found';
}

export interface AddressType {
    street: string;
    city: string;
}

export interface TestUserType {
    id: number;
    name: string;
    age: number;
    address?: AddressType;
}

export type UserPick = Pick<TestUserType, 'name' | 'address'>;
export type UserOmit = Omit<TestUserType, 'id' | 'age'>;
export type UserPartial = Partial<TestUserType>;

export function fnWithSubsetPick(obj: UserPick): object {
    return obj.address ? obj.address.city : 'Address not found';
}

export function fnWithSubsetOmit(obj: UserOmit): object {
    return obj.address ? obj.address.city : 'Address not found';
}

export function fnWithSubsetPartial(obj: UserPartial): object {
    return obj.address ? obj.address.city : 'Address not found';
}

export type UnionArrOrObj = string[] | {[key: string]: any};

export function fnWithUnionParam(obj?: UnionArrOrObj): object {
    if (obj) {
        if (Array.isArray(obj)) {
            return 'This is an array';
        }
        return 'This is an object';

    }
    return 'Argument not found';
}

export class TestUserClass {
    id:number;
    name:string;

    constructor(id: number, name: string) {
        this.id = id;
        this.name = name;
    }
}

export function fnWithUserClass(obj?: TestUserClass): object {
    return obj ? obj.name : 'Class was not passed';
}

export function fnWithUserInterface(obj?: TestUserType): object {
    return obj ? obj.name : 'Object was not passed';
}

export interface ObjectTypeWithArr {
    id: number;
    arr?: string[];
}

export function fnWithAnyParamObject(obj:ObjectTypeWithArr): object {
    return obj.arr ? obj.arr[0] : obj.id;
}
