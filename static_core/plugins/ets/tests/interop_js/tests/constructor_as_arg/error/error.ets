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
    ParentClass,
    ChildClass,
    MethodClass,
    CreateClassFunction,
} from "main_js"

const int = 1;
const str = 'str';
const foo = () => { }
const obj: object = {}
const arr: int[] = []

function checkCreateClassFunctionInt(): boolean {
    try {
        const TSClass = CreateClassFunction(int, tsInt);

        return false;
    } catch (e) {
        return true;
    }
}

function checkCreateClassFunctionString(): boolean {
    try {
        const TSClass = CreateClassFunction(str, tsInt);

        return false;
    } catch (e) {
        return true;
    }
}

function checkCreateClassFunctionObj(): boolean {
    try {
        const TSClass = CreateClassFunction(obj, tsInt);

        return false;
    } catch (e) {
        return true;
    }
}

function checkCreateClassFunctionArr(): boolean {
    try {
        const TSClass = CreateClassFunction(arr, tsInt);

        return false;
    } catch (e) {
        return true;
    }
}

function checkCreateClassFunctionArrow(): boolean {
    try {
        CreateClassFunction(foo, tsInt);

        return false;
    } catch (e) {
        return true;
    }
}

function checkCreateClassArrowFunctionInt(): boolean {
    try {
        const TSClass = CreateClassFunction(int, tsInt);

        return false;
    } catch (e) {
        return true;
    }
}

function checkCreateClassArrowFunctionString(): boolean {
    try {
        const TSClass = CreateClassFunction(str, tsInt);

        return false;
    } catch (e) {
        return true;
    }
}

function checkCreateClassArrowFunctionObj(): boolean {
    try {
        const TSClass = CreateClassFunction(obj, tsInt);

        return false;
    } catch (e) {
        return true;
    }
}

function checkCreateClassArrowFunctionArr(): boolean {
    try {
        const TSClass = CreateClassFunction(arr, tsInt);

        return false;
    } catch (e) {
        return true;
    }
}

function checkCreateClassArrowFunctionArrow(): boolean {
    try {
        CreateClassFunction(foo, tsInt);

        return false;
    } catch (e) {
        return true;
    }
}

function checkParentClassInt(): boolean {
    try {
        new ParentClass(int);

        return false;
    } catch (e) {
        return true;
    }
}

function checkParentClassString(): boolean {
    try {
        new ParentClass(str);

        return false;
    } catch (e) {
        return true;
    }
}

function checkParentClassArray(): boolean {
    try {
        new ParentClass(arr);

        return false;
    } catch (e) {
        return true;
    }
}

function checkParentClassObj(): boolean {
    try {
        new ParentClass(obj);

        return false;
    } catch (e) {
        return true;
    }
}

function checkParentClassArrow(): boolean {
    try {
        new ParentClass(foo);

        return false;
    } catch (e) {
        return true;
    }
}

function checkChildClassInt(): boolean {
    try {
        new ChildClass(int);

        return false;
    } catch (e) {
        return true;
    }
}

function checkChildClassString(): boolean {
    try {
        new ChildClass(str);

        return false;
    } catch (e) {
        return true;
    }
}

function checkChildClassArray(): boolean {
    try {
        new ChildClass(arr);

        return false;
    } catch (e) {
        return true;
    }
}

function checkChildClassObj(): boolean {
    try {
        new ChildClass(obj);

        return false;
    } catch (e) {
        return true;
    }
}

function checkChildClassArrow(): boolean {
    try {
        new ChildClass(foo);

        return false;
    } catch (e) {
        return true;
    }
}

function checkMethodClassInt(): boolean {
    try {
        const TSClass = new MethodClass();
        TSClass.init(int, int);

        return false;
    } catch (e) {
        return true;
    }
}

function checkMethodClassString(): boolean {
    try {
        const TSClass = new MethodClass();
        TSClass.init(str, int);

        return false;
    } catch (e) {
        return true;
    }
}

function checkMethodClassArr(): boolean {
    try {
        const TSClass = new MethodClass();
        TSClass.init(arr, int);

        return false;
    } catch (e) {
        return true;
    }
}

function checkMethodClassObj(): boolean {
    try {
        const TSClass = new MethodClass();
        TSClass.init(obj, int);

        return false;
    } catch (e) {
        return true;
    }
}

function checkMethodClassArrow(): boolean {
    try {
        const TSClass = new MethodClass();
        TSClass.init(foo, int);

        return false;
    } catch (e) {
        return true;
    }
}
