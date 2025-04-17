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
    ParentClass,
    ChildClass,
    AnonymousClass,
    IIFEClass,
    CreateClassFunction,
    CreateClassArrowFunction,
    checkInstance,
} from "main_js"


function checkCreateClassFunctionMain(): boolean {
    const TSClass = CreateClassFunction(MainClass, tsInt);

    return checkInstance(MainClass, TSClass) as boolean
        && TSClass._value as int == tsInt as int;
}

function checkCreateClassFunctionParent(): boolean {
    const TSClass = CreateClassFunction(ParentClass, MainClass);

    return checkInstance(ParentClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}

function checkCreateClassFunctionChild(): boolean {
    const TSClass = CreateClassFunction(ChildClass, MainClass);

    return checkInstance(ChildClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}

function checkCreateClassFunctionAnonymous(): boolean {
    const TSClass = CreateClassFunction(AnonymousClass, tsInt);
    return checkInstance(AnonymousClass, TSClass) as boolean
        && TSClass._value as int == tsInt as int;
}

function checkCreateClassFunctionIIFE(): boolean {
    const TSClass = CreateClassFunction(IIFEClass, tsInt);

    return checkInstance(IIFEClass, TSClass) as boolean
        && TSClass._value as int == tsInt as int;
}

function checkCreateClassArrowFunctionMain(): boolean {
    const TSClass = CreateClassArrowFunction(MainClass, tsInt);

    return checkInstance(MainClass, TSClass) as boolean
        && TSClass._value as int == tsInt as int;
}

function checkCreateClassArrowFunctionParent(): boolean {
    const TSClass = CreateClassArrowFunction(ParentClass, MainClass);

    return checkInstance(ParentClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}

function checkCreateClassArrowFunctionChild(): boolean {
    const TSClass = CreateClassArrowFunction(ChildClass, MainClass);

    return checkInstance(ChildClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}

function checkCreateClassArrowFunctionAnonymous(): boolean {
    const TSClass = CreateClassArrowFunction(AnonymousClass, tsInt);

    return checkInstance(AnonymousClass, TSClass) as boolean
        && TSClass._value as int == tsInt as int;
}

function checkCreateClassArrowFunctionIIFE(): boolean {
    const TSClass = CreateClassArrowFunction(IIFEClass, tsInt);

    return checkInstance(IIFEClass, TSClass) as boolean
        && TSClass._value as int == tsInt as int;
}
