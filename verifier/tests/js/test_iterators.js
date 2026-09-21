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

function *pairs(arr) {
    for (let i = 0; i + 1 < arr.length; i += 2) {
        yield [arr[i], arr[i + 1]];
    }
}

function sumPairs(arr) {
    let s = 0;
    for (const [a, b] of pairs(arr)) {
        s += a + b;
    }
    return s;
}

function entriesOf(obj) {
    let s = '';
    for (const [k, v] of Object.entries(obj)) {
        s += k + v;
    }
    return s;
}

function fromIter() {
    let arr = Array.from(pairs([1, 2, 3, 4]));
    return arr.length + arr[0][0];
}

console.log(sumPairs([1, 2, 3, 4, 5]), entriesOf({a: 1, b: 2}), fromIter());
