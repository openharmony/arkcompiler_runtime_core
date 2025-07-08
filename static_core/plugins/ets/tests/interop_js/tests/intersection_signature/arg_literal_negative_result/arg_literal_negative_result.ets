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
    tsString,
    tsNumber,
    checkLiteralType,
    AgeNameLiteralClass,
    instanceAgeNameLiteralClass,
    ChildAgeNameLiteralClass,
    instanceChildAgeNameLiteralClass,
} from 'main_js';

class IntersectionWrong {
    age = tsNumber;
}
function checkArgFuFromSts(): boolean {
    return checkLiteralType(new IntersectionWrong()) as boolean == false;
}


function checkIntersectionClass(): boolean {
    try {
        new AgeNameLiteralClass(tsNumber);

        return false;
    } catch (e) {
        return true;

    }
}

function checkIntersectionClassMethod(): boolean {
    try {
        const Intersection = new AgeNameLiteralClass(tsString, tsNumber);
        Intersection.createUser(tsNumber);

        return false;
    } catch (e) {
        return true;
    }
}
function checkInstanceIntersectionTypeClass(): boolean {
    try {
        instanceAgeNameLiteralClass.createUser(tsNumber);

        return false;
    } catch (e) {
        return true;
    }
}

function checkChildIntersectionClass(): boolean {
    try {
        new ChildAgeNameLiteralClass(tsNumber);

        return false;
    } catch (e) {
        return true;

    }
}

function checkChildIntersectionClassMethod(): boolean {
    try {
        const Intersection = new ChildAgeNameLiteralClass(tsString, tsNumber);
        Intersection.createUser(tsNumber);

        return false;
    } catch (e) {
        return true;
    }
}
function checkInstanceChildIntersectionTypeClass(): boolean {
    try {
        instanceChildAgeNameLiteralClass.createUser(tsNumber);

        return false;
    } catch (e) {
        return true;
    }
}