/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

function cons(value, tail) {
    return {value: value, tail: tail};
}

function car(node) {
    return node.value;
}

function cdr(node) {
    return node.tail;
}

function sum(a, b) {
    return a + b;
}

class TreeNode {
    value;
    left;
    right;

    constructor(value, left, right) {
        this.value = value;
        this.left = left;
        this.right = right;
    }

    sum() {
        let res = this.value;
        if (this.left) {
            res += this.left.sum();
        }
        if (this.right) {
            res += this.right.sum();
        }
        return res;
    }
}

function DynObject(x) {
    this.v0 = {value: x};
}

function Empty() {}

function make_swappable(obj) {
    obj.swap = function() {
        let tmp = this.first;
        this.first = this.second;
        this.second = tmp;
    }
}

class StaticClass {
    static staticProperty = 10;
    static staticMethod() {
        return this.staticProperty + 100;
    }
}

function extract_squared_int(obj) {
    let x = obj.int_value;
    return x * x;
}

function ObjectWithPrototype() {
    this.overridden_value = 4;
    this.overridden_function = function() {
        return "overridden";
    }
}

let o1 = new ObjectWithPrototype();

ObjectWithPrototype.prototype.overridden_value = -1;
ObjectWithPrototype.prototype.overridden_function = function() {
    return "should be overridden";
};
ObjectWithPrototype.prototype.prototype_value = 5;
ObjectWithPrototype.prototype.prototype_function = function() {
    return "prototype function";
};

let dyn_storage = {
    str: "abcd",
    dbl: 1.9,
    integer: 6,
    bool_true: true,
    bool_false: false,
    verify: function() {
        return (this.str === "dcba") + (this.dbl === 2.4) + (this.integer === 31) + (this.bool_true === false);
    }
};

exports.cons = cons;
exports.car = car;
exports.cdr = cdr;
exports.sum = sum;
exports.TreeNode = TreeNode;
exports.DynObject = DynObject;
exports.Empty = Empty;
exports.make_swappable = make_swappable;
exports.StaticClass = StaticClass;
exports.extract_squared_int = extract_squared_int;
exports.ObjectWithPrototype = ObjectWithPrototype;
exports.dyn_storage = dyn_storage;
exports.vundefined = undefined;
