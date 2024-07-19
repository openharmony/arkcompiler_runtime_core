/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
    ts_int,
    ParentClass,
    ChildClass,
    MethodClass,
    create_class_function,
} from "main_js"

const int = 1;
const str = 'str';
const foo = () => { }
const obj: object = {}
const arr: int[] = []

function check_create_class_function_int(): boolean {
    try {
        const TSClass = create_class_function(int, ts_int);

        return false;
    } catch (e) {
        return true;
    }
}

function check_create_class_function_string(): boolean {
    try {
        const TSClass = create_class_function(str, ts_int);

        return false;
    } catch (e) {
        return true;
    }
}

function check_create_class_function_obj(): boolean {
    try {
        const TSClass = create_class_function(obj, ts_int);

        return false;
    } catch (e) {
        return true;
    }
}

function check_create_class_function_arr(): boolean {
    try {
        const TSClass = create_class_function(arr, ts_int);

        return false;
    } catch (e) {
        return true;
    }
}

function check_create_class_function_arrow(): boolean {
    try {
        create_class_function(foo, ts_int);

        return false;
    } catch (e) {
        return true;
    }
}

function check_create_class_arrow_function_int(): boolean {
    try {
        const TSClass = create_class_function(int, ts_int);

        return false;
    } catch (e) {
        return true;
    }
}

function check_create_class_arrow_function_string(): boolean {
    try {
        const TSClass = create_class_function(str, ts_int);

        return false;
    } catch (e) {
        return true;
    }
}

function check_create_class_arrow_function_obj(): boolean {
    try {
        const TSClass = create_class_function(obj, ts_int);

        return false;
    } catch (e) {
        return true;
    }
}

function check_create_class_arrow_function_arr(): boolean {
    try {
        const TSClass = create_class_function(arr, ts_int);

        return false;
    } catch (e) {
        return true;
    }
}

function check_create_class_arrow_function_arrow(): boolean {
    try {
        create_class_function(foo, ts_int);

        return false;
    } catch (e) {
        return true;
    }
}

function check_parent_class_int(): boolean {
    try {
        new ParentClass(int);

        return false;
    } catch (e) {
        return true;
    }
}

function check_parent_class_string(): boolean {
    try {
        new ParentClass(str);

        return false;
    } catch (e) {
        return true;
    }
}

function check_parent_class_array(): boolean {
    try {
        new ParentClass(arr);

        return false;
    } catch (e) {
        return true;
    }
}

function check_parent_class_obj(): boolean {
    try {
        new ParentClass(obj);

        return false;
    } catch (e) {
        return true;
    }
}

function check_parent_class_arrow(): boolean {
    try {
        new ParentClass(foo);

        return false;
    } catch (e) {
        return true;
    }
}

function check_child_class_int(): boolean {
    try {
        new ChildClass(int);

        return false;
    } catch (e) {
        return true;
    }
}

function check_child_class_string(): boolean {
    try {
        new ChildClass(str);

        return false;
    } catch (e) {
        return true;
    }
}

function check_child_class_array(): boolean {
    try {
        new ChildClass(arr);

        return false;
    } catch (e) {
        return true;
    }
}

function check_child_class_obj(): boolean {
    try {
        new ChildClass(obj);

        return false;
    } catch (e) {
        return true;
    }
}

function check_child_class_arrow(): boolean {
    try {
        new ChildClass(foo);

        return false;
    } catch (e) {
        return true;
    }
}

function check_method_class_int(): boolean {
    try {
        const TSClass = new MethodClass();
        TSClass.init(int, int);

        return false;
    } catch (e) {
        return true;
    }
}

function check_method_class_string(): boolean {
    try {
        const TSClass = new MethodClass();
        TSClass.init(str, int);

        return false;
    } catch (e) {
        return true;
    }
}

function check_method_class_arr(): boolean {
    try {
        const TSClass = new MethodClass();
        TSClass.init(arr, int);

        return false;
    } catch (e) {
        return true;
    }
}

function check_method_class_obj(): boolean {
    try {
        const TSClass = new MethodClass();
        TSClass.init(obj, int);

        return false;
    } catch (e) {
        return true;
    }
}

function check_method_class_arrow(): boolean {
    try {
        const TSClass = new MethodClass();
        TSClass.init(foo, int);

        return false;
    } catch (e) {
        return true;
    }
}
