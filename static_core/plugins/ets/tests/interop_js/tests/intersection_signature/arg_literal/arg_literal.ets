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
    checkLiteralType,
    AgeNameLiteralClass,
    createAgeNameLiteralClassFromTs,
    callMethodAgeNameLiteralClassFromTs,
    instanceAgeNameLiteralClass,
    ChildAgeNameLiteralClass,
    createChildAgeNameLiteralClassFromTs,
    callMethodChildAgeNameLiteralClassFromTs,
    instanceChildAgeNameLiteralClass,
} from 'main_js';


class Intersection {
    age = tsNumber;
    name = tsString;
}
function checkArgFuFromSts(): boolean {

    return checkLiteralType(new Intersection()) as boolean;
}

function checkIntersectionTypeClass(): boolean {
    new AgeNameLiteralClass(tsString, tsNumber);

    return true;
}

function checkIntersectionClassMethod(): boolean {
    const Intersection = new AgeNameLiteralClass(tsString, tsNumber);
    Intersection.createUser(tsString, tsNumber);

    return true;
}

function checkCreateIntersectionClassTypeFromTs(): boolean {
    return createAgeNameLiteralClassFromTs() as boolean;
}

function checkCallMethodIntersectionClassTypeFromTs(): boolean {
    return callMethodAgeNameLiteralClassFromTs() as boolean;
}

function checkInstanceIntersectionTypeClass(): boolean {
    instanceAgeNameLiteralClass.createUser(tsString, tsNumber);
    return true;
}

function checkChildIntersectionTypeClass(): boolean {
    new ChildAgeNameLiteralClass(tsString, tsNumber);

    return true;
}

function checkChildIntersectionTypeClassMethod(): boolean {
    const Intersection = new ChildAgeNameLiteralClass(tsString, tsNumber);
    Intersection.createUser(tsString, tsNumber);

    return true;
}

function checkCreateChildIntersectionClassTypeFromTs(): boolean {
    return createChildAgeNameLiteralClassFromTs() as boolean;
}

function checkCallMethodChildIntersectionClassTypeFromTs(): boolean {
    return callMethodChildAgeNameLiteralClassFromTs() as boolean;
}

function checkInstanceChildIntersectionTypeClass(): boolean {
    instanceChildAgeNameLiteralClass.createUser(tsString, tsNumber);
    return true;
}
