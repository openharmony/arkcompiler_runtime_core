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

function *count(n) {
    let i = 0;
    while (i < n) {
        yield i;
        i++;
    }
}

function *range(from, to, step) {
    for (let i = from; i < to; i += step) {
        yield i;
    }
}

async function *asyncCount(n) {
    let i = 0;
    while (i < n) {
        yield i;
        i++;
    }
}

function sumGen(g) {
    let s = 0;
    for (const v of g) {
        s += v;
    }
    return s;
}

let sum = sumGen(count(4)) + sumGen(range(1, 5, 1));
asyncCount(2).next().then((r) => {
    console.log(sum, r.value);
});
