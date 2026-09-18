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

function classify(n) {
    switch (n) {
        case 0:
            return 'zero';
        case 1:
        case 2:
            return 'small';
        default:
            return n < 0 ? 'neg' : 'big';
    }
}

function sumRange(n) {
    let s = 0;
    for (let i = 0; i < n; i++) {
        if (i % 2 === 0) {
            s += i;
            continue;
        }
        s -= 1;
    }
    return s;
}

function walkArr(arr) {
    let s = 0;
    for (const v of arr) {
        s += v;
    }
    for (const k in arr) {
        s += Number(k);
    }
    let i = arr.length;
    while (i > 0) {
        i--;
        s += arr[i];
    }
    return s;
}

function untilNeg(start) {
    let x = start;
    do {
        x -= 3;
    } while (x > 0);
    return x;
}

console.log(classify(0), classify(2), classify(9), classify(-1));
console.log(sumRange(8), walkArr([1, 2, 3]), untilNeg(10));
