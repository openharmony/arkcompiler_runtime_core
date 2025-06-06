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
    MethodClass,
    ParentClass,
    ChildClass,
    AnonymousClass,
    IIFEClass,
    checkInstance,
} from "main_js"


function checkMethodConstructorMainClass(): boolean {
    const MClass = new MethodClass();
    const TSClass = MClass.init(MainClass, tsInt);


    return checkInstance(MainClass, TSClass) as boolean
        && TSClass._value as int == tsInt as int;
}

function checkMethodClassParentClass(): boolean {
    const MClass = new MethodClass();
    const TSClass = MClass.init(ParentClass, MainClass);
    return checkInstance(ParentClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}

function checkMethodClassChildClass(): boolean {
    const MClass = new MethodClass();
    const TSClass = MClass.init(ChildClass, MainClass);
    return checkInstance(ChildClass, TSClass) as boolean
        && TSClass._otherClass._value as int == tsInt as int;
}

function checkMethodClassAnonymousClass(): boolean {
    const MClass = new MethodClass();
    const TSClass = MClass.init(AnonymousClass, tsInt);

    return checkInstance(AnonymousClass, TSClass) as boolean
        && TSClass._value as int == tsInt as int;
}

function checkMethodClassIIFEClass(): boolean {
    const MClass = new MethodClass();
    const TSClass = MClass.init(IIFEClass, tsInt);

    return checkInstance(IIFEClass, TSClass) as boolean
        && TSClass._value as int == tsInt as int;
}
