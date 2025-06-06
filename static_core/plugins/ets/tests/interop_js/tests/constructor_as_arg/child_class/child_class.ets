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
    tsInt,
    MainClass,
    AnonymousClass,
    ChildClass,
    createChildClassWithArgFromTs,
    createChildClassWithMainFromTs,
    childClassWithMainInstance,
    createChildClassWithAnonymousFromTs,
    childClassWithAnonymousInstance,
    createChildClassWithIIFEFromTs,
    childClassWithIIFEInstance,
    IIFEClass,
    checkInstance,
} from "main_js"

function checkInheritorClassMain(): boolean {
    const TSClass = new ChildClass(MainClass);

    return checkInstance(ChildClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}

function checkInheritorClassAnonymous(): boolean {
    const TSClass = new ChildClass(AnonymousClass);

    return checkInstance(ChildClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}

function checkInheritorClassIIFE(): boolean {
    const TSClass = new ChildClass(IIFEClass);

    return checkInstance(ChildClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}

function checkCreateChildClassWithArgMainFromTs(): boolean {
    const TSClass = createChildClassWithArgFromTs(MainClass);

    return checkInstance(ChildClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}

function checkCreateChildClassWithArgAnonymousFromTs(): boolean {
    const TSClass = createChildClassWithArgFromTs(AnonymousClass);

    return checkInstance(ChildClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}

function checkCreateChildClassWithArgIIFEFromTs(): boolean {
    const TSClass = createChildClassWithArgFromTs(IIFEClass);

    return checkInstance(ChildClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}

function checkCreateChildClassWithMainFromTs(): boolean {
    const TSClass = createChildClassWithMainFromTs();

    return checkInstance(ChildClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}

function checkChildClassWithMainInstance(): boolean {
    const TSClass = childClassWithMainInstance;

    return checkInstance(ChildClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}

function checkCreateChildClassWithAnonymousFromTs(): boolean {
    const TSClass = createChildClassWithAnonymousFromTs();

    return checkInstance(ChildClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}

function checkChildClassWithAnonymousInstance(): boolean {
    const TSClass = childClassWithAnonymousInstance;

    return checkInstance(ChildClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}

function checkCreateChildClassWithIIFEFromTs(): boolean {
    const TSClass = createChildClassWithIIFEFromTs();

    return checkInstance(ChildClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}

function checkChildClassWithIIFEInstance(): boolean {
    const TSClass = childClassWithIIFEInstance;

    return checkInstance(ChildClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}
