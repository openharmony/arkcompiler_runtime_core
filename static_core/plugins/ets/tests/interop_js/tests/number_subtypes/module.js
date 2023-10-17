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
"use strict";

function EvaluateNumber(v0, v1) {
    return v0 + v1;
}

function EmptyFunction() {
    console.log("Hello from empty function");
}

function EvaluateObject(obj) {
    return new ExampleClass(obj.v0 + obj.v1, obj.v0 + obj.v1);
}

function EvaluateArray(arr, size) {
    let result = [];
    for (let i = 0; i < size; i++) {
        result[i] = arr[i] + i * i;
    }
    return result;
}

class ExampleClass {
    constructor(v0, v1) {
        this.v0 = v0;
        this.v1 = v1;
    }

    static EvaluateNumber(v2, v3) {
        return v2 + v3;
    }

    static EvaluateArray(arr, size) {
        let result = [];
        for (let i = 0; i < size; i++) {
            result[i] = arr[i] + i * i;
        }
        return result;
    }

    InstanceEvaluateNumber() {
        // without "this" also not working
        return this.v0 + this.v1;
    }

    EvaluateObject(obj) {
        return new ExampleClass(obj.v0 + this.v1, this.v0 + obj.v1);
    }

    GetV0() {
        return this.v0
    }

    GetV1() {
        return this.v1
    }

    static EmptyMethod() {
        console.log("Hello from empty method");
    }
};

class ClassWithEmptyConstructor {
    constructor() {
        this.v0 = 42;
        this.v1 = 42;
    }

    varify_properties(v0_expected, v1_expected) {
        if (this.v0 == v0_expected && this.v1 == v1_expected) {
            return 0;
        } else {
            return 1;
        }
    }

    GetV0() {
        return this.v0
    }

    GetV1() {
        return this.v1
    }
}

// this is TS namespace
// export namespace MyNamespace {
//     class Kitten {
//         constructor(id: number, name: string) {
//             this.id = id;
//             this.name = name;
//         }

//         id: number;
//         name: string;
//     }

//     export function create_kitten(id: number, name: string) {
//         return new Kitten(id, name);
//     }
// }

var MyNamespace;
(function (MyNamespace) {
    var Kitten = /** @class */ (function () {
        function Kitten(id, name) {
            this.id = id;
            this.name = name;
        }
        return Kitten;
    }());
    function create_kitten(id, name) {
        return new Kitten(id, name);
    }
    MyNamespace.create_kitten = create_kitten;
})(MyNamespace = exports.MyNamespace || (exports.MyNamespace = {}));

exports.EvaluateNumber = EvaluateNumber;
exports.ExampleClass = ExampleClass;
exports.EmptyFunction = EmptyFunction;
exports.EvaluateObject = EvaluateObject;
exports.ClassWithEmptyConstructor = ClassWithEmptyConstructor;
exports.EvaluateArray = EvaluateArray;
