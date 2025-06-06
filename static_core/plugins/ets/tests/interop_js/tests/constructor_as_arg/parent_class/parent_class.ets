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
    ParentClass,
    createClassWithArgFromTs,
    createMainClassFromTs,
    createAnonymousClassFromTs,
    anonymousClassInstance,
    createIIFEClassFromTs,
    iifeClassInstance,
    mainClassInstance,
    IIFEClass,
    checkInstance,
} from "main_js"

function checkMainClassAsArg(): boolean {
    const TSClass = new ParentClass(MainClass);

    return checkInstance(ParentClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}

function checkAnonymousClassAsArg(): boolean {
    const TSClass = new ParentClass(AnonymousClass);

    return checkInstance(ParentClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}

function checkIIFEClassAsArg(): boolean {
    const TSClass = new ParentClass(IIFEClass);

    return checkInstance(ParentClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}

function checkCreateMainClassWithArgFromTs(): boolean {
    const TSClass = createClassWithArgFromTs(MainClass);

    return checkInstance(ParentClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}

function checkCreateMainClassFromTs(): boolean {
    const TSClass = createMainClassFromTs();

    return checkInstance(ParentClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}

function checkMainClassInstance(): boolean {
    const TSClass = mainClassInstance;
    return checkInstance(ParentClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}

function checkCreateAnonymousClassWithArgFromTs(): boolean {
    const TSClass = createClassWithArgFromTs(AnonymousClass);

    return checkInstance(ParentClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}

function checkCreateAnonymousClassFromTs(): boolean {
    const TSClass = createAnonymousClassFromTs();
    return checkInstance(ParentClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}

function checkAnonymousClassInstance(): boolean {
    const TSClass = anonymousClassInstance;

    return checkInstance(ParentClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}

function checkCreateIifeClassFromTs(): boolean {
    const TSClass = createIIFEClassFromTs();

    return checkInstance(ParentClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}

function checkIifeClassInstance(): boolean {
    const TSClass = iifeClassInstance;

    return checkInstance(ParentClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}
