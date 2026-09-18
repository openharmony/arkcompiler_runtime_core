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

function nestedTry(flag) {
    try {
        try {
            if (flag) {
                throw 1;
            }
            return 0;
        } catch (inner) {
            throw inner + 1;
        }
    } catch (outer) {
        return outer;
    } finally {
        flag = false;
    }
}

function withFinally() {
    let x = 0;
    try {
        x = 1;
        throw 2;
    } finally {
        x = 3;
    }
    return x;
}

function catchOrder(n) {
    try {
        if (n < 0) {
            throw 'neg';
        }
        if (n === 0) {
            throw 0;
        }
        return n;
    } catch (e) {
        return typeof e === 'string' ? -1 : 0;
    }
}

function recover(fn) {
    try {
        return fn();
    } catch (e) {
        return e;
    }
}

console.log(nestedTry(true), nestedTry(false));
console.log(catchOrder(-1), catchOrder(0), catchOrder(2));
try {
    withFinally();
} catch (e) {
    console.log(e, recover(() => {
        throw 7;
    }));
}
