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
    checkIntersectionInterface,
    AgeNameInterfaceClass,
    instanceAgeNameInterfaceClass,
    ChildAgeNameInterfaceClass,
    instanceChildAgeNameInterfaceClass,
} from 'main_js';

class AgeNameWrong {
    age = tsNumber;
}
function checkArgFuFromSts(): boolean {
    return checkIntersectionInterface(new AgeNameWrong()) as boolean == false;
}

function checkAgeNameClass(): boolean {
    try {
        new AgeNameInterfaceClass(tsNumber);

        return false;
    } catch (e) {
        return true;

    }
}

function checkAgeNameClassMethod(): boolean {
    try {
        const AgeName = new AgeNameInterfaceClass(tsString, tsNumber);
        AgeName.createUser(tsNumber);

        return false;
    } catch (e) {
        return true;
    }
}
function checkInstanceAgeNameInterfaceClass(): boolean {
    try {
        instanceAgeNameInterfaceClass.createUser(tsNumber);

        return false;
    } catch (e) {
        return true;
    }
}

function checkChildAgeNameClass(): boolean {
    try {
        new ChildAgeNameInterfaceClass(tsNumber);

        return false;
    } catch (e) {
        return true;

    }
}

function checkChildAgeNameClassMethod(): boolean {
    try {
        const AgeName = new ChildAgeNameInterfaceClass(tsString, tsNumber);
        AgeName.createUser(tsNumber);

        return false;
    } catch (e) {
        return true;
    }
}
function checkInstanceChildAgeNameInterfaceClass(): boolean {
    try {
        instanceChildAgeNameInterfaceClass.createUser(tsNumber);

        return false;
    } catch (e) {
        return true;
    }
}
