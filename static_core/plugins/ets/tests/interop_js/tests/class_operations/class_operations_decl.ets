/**
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
    EvaluateNumber,
    ExampleClass,
    ClassWithEmptyConstructor,
    EvaluateObject,
    MyNamespace,
    EvaluateArray,
} from "/plugins/ets/tests/interop_js/tests/number_subtypes/module"

function jscall_empty(): int {
    EvaluateNumber();
    return 0;
}

function jsnew_empty(): int {
    let res_obj = new ClassWithEmptyConstructor();
    let res_get_v0 = res_obj.v0 as number;
    let res_get_v1 = res_obj.v1 as number;
    if (res_get_v0 == 42 && res_get_v1 == 42) {
        return 0;
    } else {
        return 1;
    }
}

function jscall_static_method_empty(): int {
    ExampleClass.EmptyMethod();
    return 0;
}

function jscall_object(): int {
    let res_obj = new ExampleClass(42, 42);
    let res = EvaluateObject(res_obj);
    let res_get_v0 = res.v0 as number;
    let res_get_v1 = res.v1 as number;
    if (res_get_v0 == 84 && res_get_v1 == 84) {
        return 0;
    } else {
        return 1;
    }
}

function jsnew_object(): int {
    let res_obj = new ExampleClass(42, 42);
    let res = new ExampleClass(res_obj, res_obj);
    let res_get_v0 = res.v0 as ExampleClass;
    let res_get_v1 = res.v1 as ExampleClass;
    if (res_get_v0.v0 as number == 42 && res_get_v0.v1 as number == 42 &&
        res_get_v1.v0 as number == 42 && res_get_v1.v1 as number == 42) {
        return 0;
    } else {
        return 1;
    }
}

function jsnew_setproperty_object(): int {
    let res_obj = new ClassWithEmptyConstructor();
    let res = new ClassWithEmptyConstructor();
    res_obj.v0 = res;
    res_obj.v1 = res;
    let res_get_v0 = res_obj.v0 as ClassWithEmptyConstructor;
    let res_get_v1 = res_obj.v1 as ClassWithEmptyConstructor;
    if (res_get_v0.v0 as number == 42 && res_get_v0.v1 as number == 42 &&
        res_get_v1.v0 as number == 42 && res_get_v1.v1 as number == 42) {
        return 0;
    } else {
        return 1;
    }
}

function jscall_method_object(): int {
    let res_obj = new ExampleClass(42, 42);
    let res = EvaluateObject(res_obj);
    let res_get_v0 = res.v0 as number;
    let res_get_v1 = res.v1 as number;
    if (res_get_v0 == 84 && res_get_v1 == 84) {
        return 0;
    } else {
        return 1;
    }
}

function jscall_method_simple(): int {
    let res_obj = new ExampleClass(42, 42);
    let res = res_obj.InstanceEvaluateNumber() as number;
    if (res == 84) {
        return 0;
    } else {
        return 1;
    }
}

function jscall_string(): int {
    let v0: String = "hello";
    let v1: String = "world";
    let res = EvaluateNumber(v0, v1);
    if (res == "helloworld") {
        return 0;
    } else {
        return 1;
    }
}

function jsnew_string(): int {
    let v0: String = "hello";
    let v1: String = "world";
    let res_obj = new ExampleClass(v0, v1);
    let res_get_v0 = res_obj.v0 as String;
    let res_get_v1 = res_obj.v1 as String;
    if (res_get_v0 == v0 && res_get_v1 == v1) {
        return 0;
    } else {
        return 1;
    }
}

function jsnew_setproperty_string(): int {
    let v0: String = "hello";
    let v1: String = "world";
    let res_obj = new ClassWithEmptyConstructor();
    res_obj.v0 = v0;
    res_obj.v1 = v1;
    let res_get_v0 = res_obj.v0 as String;
    let res_get_v1 = res_obj.v1 as String;
    if (res_get_v0 == v0 && res_get_v1 == v1) {
        return 0;
    } else {
        return 1;
    }
}

function jscall_static_method_string(): int {
    let v0: String = "hello";
    let v1: String = "world";
    let res = ExampleClass.EvaluateNumber(v0, v1);
    if (res == "helloworld") {
        return 0;
    } else {
        return 1;
    }
}

function jscall_array(): int {
    let v0: int[] = new int[10];
    for (let i = 0; i < 10; ++i) {
        v0[i] = i * i;
    }

    let res = EvaluateArray(v0, 10);
    for (let i = 0; i < 10; ++i) {
        if (res[i] != 2 * i * i) {
            return 1;
        }
    }

    return 0;
}

function jsnew_array(): int {
    let v0: int[] = new int[10];
    for (let i = 0; i < 10; ++i) {
        v0[i] = i * i;
    }

    let res_obj = new ExampleClass(v0, v0);
    let res_get_v0 = res_obj.v0;
    let res_get_v1 = res_obj.v1;

    for (let i = 0; i < 10; ++i) {
        if (res_get_v0[i] as int != v0[i] || res_get_v1[i] as int != v0[i]) {
            return 1;
        }
    }
    return 0;
}

function jsnew_setproperty_array(): int {
    let v0: int[] = new int[10];
    for (let i = 0; i < 10; ++i) {
        v0[i] = i * i;
    }

    let res_obj = new ClassWithEmptyConstructor();

    res_obj.v0 = v0;
    res_obj.v1 = v0;
    let res_get_v0 = res_obj.v0;
    let res_get_v1 = res_obj.v1;

    for (let i = 0; i < 10; ++i) {
        if (res_get_v0[i] as int != v0[i] || res_get_v1[i] as int != v0[i]) {
            return 1;
        }
    }
    return 0;
}

function jscall_static_method_array(): int {
    let v0: int[] = new int[10];
    for (let i = 0; i < 10; ++i) {
        v0[i] = i * i;
    }

    let res = ExampleClass.EvaluateArray(v0, 10);
    for (let i = 0; i < 10; ++i) {
        if (res[i] != 2 * i * i) {
            return 1;
        }
    }
    return 0;
}

function test_namespace(): int {
    let v0: int = 1;
    let v1: String = "Pinky";
    let res_obj = MyNamespace.create_kitten(v0, v1);
    let res_id = res_obj.id;
    let res_name = res_obj.name;
    if (res_id == v0 && res_name == v1) {
        return 0;
    } else {
        return 1;
    }
}
