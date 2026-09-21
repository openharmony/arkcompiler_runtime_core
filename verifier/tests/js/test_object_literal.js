// Copyright (c) 2026 Huawei Device Co., Ltd.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

const base = {a: 1, b: 2, c: 3};
const extra = {d: 4, e: 5};
const merged = {...base, ...extra, f: 6};
const {a, b, ...rest} = merged;
const keys = Object.keys(rest);
const opt = merged?.e ?? 0;
const missing = merged?.z?.y ?? -1;

function pick(obj) {
    return obj?.a + (obj?.missing ?? 0);
}

function rename({a: x, b: y}) {
    return x + y;
}

const computed = {
    ['k' + 1]: 10,
    ['k' + 2]: 20,
};

function mergeAll(left, right) {
    return {...left, ...right, n: (left.n ?? 0) + 1};
}

console.log(a, b, keys.length, opt, missing, pick(base));
console.log(rename(base), computed.k1, mergeAll(base, extra).n);
