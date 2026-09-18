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

function add(a, b, c) {
    return a + b + c;
}

function callSpread(arr) {
    return add(...arr);
}

function restSum(head, ...tail) {
    let s = head;
    for (const v of tail) {
        s += v;
    }
    return s;
}

function unpack(point) {
    let [x, y, z = 0] = point;
    let {n, m = 1} = {n: x + y};
    return x + y + z + n + m;
}

function swap(a, b) {
    [a, b] = [b, a];
    return a - b;
}

console.log(callSpread([1, 2, 3]), restSum(1, 2, 3, 4), unpack([5, 6]), swap(2, 9));
