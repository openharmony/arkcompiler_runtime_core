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
    AgeNameInterfaceClass,
    checkIntersectionInterface,
    createAgeNameClassInterfaceFromTs,
    callMethodAgeNameClassInterfaceFromTs,
    instanceAgeNameInterfaceClass,
    ChildAgeNameInterfaceClass,
    createChildAgeNameClassInterfaceFromTs,
    callMethodChildAgeNameClassInterfaceFromTs,
    instanceChildAgeNameInterfaceClass,
} from 'main_js';

class AgeNameClass {
    age = tsNumber;
    name = tsString;
}
function checkArgFuFromSts(): boolean {
    return checkIntersectionInterface(new AgeNameClass()) as boolean;
}

function checkAgeNameClass(): boolean {
    new AgeNameInterfaceClass(tsString, tsNumber);

    return true;
}

function checkAgeNameClassMethod(): boolean {
    const AgeName = new AgeNameInterfaceClass(tsString, tsNumber);
    AgeName.createUser(tsString, tsNumber);

    return true;
}

function checkCreateAgeNameClassInterfaceFromTs(): boolean {
    return createAgeNameClassInterfaceFromTs() as boolean;
}

function checkCallMethodAgeNameClassInterfaceFromTs(): boolean {
    return callMethodAgeNameClassInterfaceFromTs() as boolean;
}

function checkInstanceAgeNameInterfaceClass(): boolean {
    instanceAgeNameInterfaceClass.createUser(tsString, tsNumber);
    return true;
}

function checkChildAgeNameInterfaceClass(): boolean {
    const AgeName = new ChildAgeNameInterfaceClass(tsString, tsNumber);

    return true;
}

function checkChildAgeNameInterfaceClassMethod(): boolean {
    const AgeName = new ChildAgeNameInterfaceClass(tsString, tsNumber);
    AgeName.createUser(tsString, tsNumber);

    return true;
}

function checkCreateChildAgeNameClassInterfaceFromTs(): boolean {
    return createChildAgeNameClassInterfaceFromTs() as boolean;
}

function checkCallMethodChildAgeNameClassInterfaceFromTs(): boolean {
    return callMethodChildAgeNameClassInterfaceFromTs() as boolean;
}

function checkInstanceChildAgeNameInterfaceClass(): boolean {
    instanceChildAgeNameInterfaceClass.createUser(tsString, tsNumber);
    return true;
}