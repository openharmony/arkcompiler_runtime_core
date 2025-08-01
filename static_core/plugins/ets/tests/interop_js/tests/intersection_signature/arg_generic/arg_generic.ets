/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
    tsNumber,
    tsString,
    checkAgeNameGeneric,
    AgeNameGenericClass,
    createAgeNameClassGenericFromTs,
    callMethodAgeNameClassGenericFromTs,
    instanceAgeNameGenericClass,
    ChildAgeNameGenericClass,
    createChildAgeNameClassGenericFromTs,
    callMethodChildAgeNameClassGenericFromTs,
    instanceChildAgeNameGenericClass,
} from 'main_js';

class TypeClass {
    age = tsNumber;
    name = tsString;
}
function checkArgFuFromSts(): boolean {
    return checkAgeNameGeneric(new TypeClass()) as boolean;
}

function checkTypeClass(): boolean {
    new AgeNameGenericClass(tsString, tsNumber);

    return true;
}

function checkClassMethod(): boolean {
    const Type = new AgeNameGenericClass(tsString, tsNumber);
    Type.createUser(tsString, tsNumber);

    return true;
}

function checkCreateClassFromTs(): boolean {
    return createAgeNameClassGenericFromTs() as boolean;
}

function checkCallMethodClassFromTs(): boolean {
    return callMethodAgeNameClassGenericFromTs() as boolean;
}

function checkInstanceClass(): boolean {
    instanceAgeNameGenericClass.createUser(tsString, tsNumber);
    return true;
}

function checkChildClass(): boolean {
    const Type = new ChildAgeNameGenericClass(tsString, tsNumber);

    return true;
}

function checkChildClassMethod(): boolean {
    const Type = new ChildAgeNameGenericClass(tsString, tsNumber);
    Type.createUser(tsString, tsNumber);

    return true;
}

function checkCreateChildClassFromTs(): boolean {
    return createChildAgeNameClassGenericFromTs() as boolean;
}

function checkCallMethodChildClassFromTs(): boolean {
    return callMethodChildAgeNameClassGenericFromTs() as boolean;
}

function checkInstanceChildClass(): boolean {
    instanceChildAgeNameGenericClass.createUser(tsString, tsNumber);
    return true;
}