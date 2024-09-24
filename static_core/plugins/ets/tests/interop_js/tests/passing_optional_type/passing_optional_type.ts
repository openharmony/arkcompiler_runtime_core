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
export function fnWithAnyParams(name:string, surname?:{}):string {
    return surname ? `${name}; ${surname}` : name;
}

export interface ObjectTypeWithAny {
    name:string;
    surname?:{};
}

export function fnWithAnyParamsObject(obj:ObjectTypeWithAny):string {
    return obj.surname ? `${obj.name}; ${obj.surname}` : obj.name;
}


export function fnWithLiteralParam(name:string, surname?:'Smith' | 'Dou'):string {
     return surname ? `${name}; ${surname}` : name;
}

export interface ObjectTypeWithLiteral {
    name:string;
    surname?:'Smith' | 'Dou';
}

export function fnWithLiteralObjectParam(obj:ObjectTypeWithLiteral):string {
    return obj.surname ? `${obj.name}; ${obj.surname}` : obj.name;
}

export function fnWithExtraSetParam(name:string, surname?:unknown):string {
    return surname ? `${name}; ${surname}` : name;
}

export interface ObjectTypeWithExtraSet {
    name:string;
    surname?:unknown;
}

export function fnWithExtraSetObjectParam(obj:ObjectTypeWithExtraSet):string {
    return obj.surname ? `${obj.name}; ${obj.surname}` : obj.name;
}

export interface TestUserType {
    name:string;
    id?:number | string;
    city?:string;
}

type BasicTestUser = Pick<TestUserType, 'name' | 'city' >;

export function fnWithSubSetParam(obj:BasicTestUser):string {
    return obj.city ? `${obj.name}; ${obj.city}` : obj.name;
}

export function fnWithUnionParam(id?:string | number):string | number {
    return id ? id : 'id not found';
}

export function fnWithUnionObjectParam(obj:TestUserType):string | number {
    return obj.id ? obj.id : 'id not found';
}

export interface TestSecondUserType {
    name:string;
    surname?: string;
    id:number;
    city:string;
}

type TestUserTypeReduseProperty = Omit<TestSecondUserType, 'id' | 'city' >;

export function fnWithSubSetReduseParam(obj:TestUserTypeReduseProperty):string {
    return obj.surname ? `${obj.name}; ${obj.surname}` : obj.name;
}

type PartialTestUserType = Partial<TestSecondUserType>;

export function fnWithSubSetPartialParam(obj:PartialTestUserType):string | undefined {
    return obj.surname ? `${obj.name}; ${obj.surname}` : obj.name;
}