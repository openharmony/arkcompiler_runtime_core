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
    checkUnionType,
    AgeNameUnionClass,
    createAgeNameUnionClassFromTs,
    callMethodAgeNameUnionClassFromTs,
    instanceAgeNameUnionClass,
    ChildAgeNameUnionClass,
    createChildAgeNameUnionClassFromTs,
    callMethodChildAgeNameUnionClassFromTs,
    instanceChildAgeNameUnionClass,
} from 'main_js';

class Intersection1 {
    age = tsNumber;
    name = tsString;
}
function checkArgFuFromSts(): boolean {
    return checkUnionType(new Intersection1()) as boolean;
}

function checkIntersectionTypeClass(): boolean {
    new AgeNameUnionClass(tsString, tsNumber);

    return true;
}

function checkIntersectionClassMethod(): boolean {
    const Intersection = new AgeNameUnionClass(tsString, tsNumber);
    Intersection.createUser(tsString, tsNumber);

    return true;
}

function checkCreateIntersectionClassTypeFromTs(): boolean {
    return createAgeNameUnionClassFromTs() as boolean;
}

function checkCallMethodIntersectionClassTypeFromTs(): boolean {
    return callMethodAgeNameUnionClassFromTs() as boolean;
}

function checkInstanceIntersectionTypeClass(): boolean {
    instanceAgeNameUnionClass.createUser(tsString, tsNumber);
    return true;
}

function checkChildIntersectionTypeClass(): boolean {
    new ChildAgeNameUnionClass(tsString, tsNumber);

    return true;
}

function checkChildIntersectionTypeClassMethod(): boolean {
    const Intersection = new ChildAgeNameUnionClass(tsString, tsNumber);
    Intersection.createUser(tsString, tsNumber);

    return true;
}

function checkCreateChildIntersectionClassTypeFromTs(): boolean {
    return createChildAgeNameUnionClassFromTs() as boolean;
}

function checkCallMethodChildIntersectionClassTypeFromTs(): boolean {
    return callMethodChildAgeNameUnionClassFromTs() as boolean;
}

function checkInstanceChildIntersectionTypeClass(): boolean {
    instanceChildAgeNameUnionClass.createUser(tsString, tsNumber);
    return true;
}
