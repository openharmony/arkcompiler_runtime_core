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

let etsVm = gtest.etsVm;
exports.etsVm = etsVm;

exports.Nop = function () { }
exports.Identity = function (v) { return v }
exports.StringifyValue = function (v) { return typeof v + ":" + v }
exports.StringifyArgs = function (...args) { return args.toString() }
exports.GetProtoConstructor = function (v) { return Object.getPrototypeOf(v).constructor }
exports.ApplyArgs = function (fn, ...args) { return fn(...args) }
exports.ThrowValue = function (v) { throw v; }
exports.Log = function (...args) { console.log(...args) }
exports.GetProp = function (v, p) { return v[p] }

exports.Sum = function (a, b) { return a + b }
exports.Car = function (v) { this.color = v }

exports.TestProxy = function () {
    Object.defineProperty(this, "foo", {
        get: function () { throw Error("get exception") },
        set: function () { throw Error("set exception") }
    });
}
