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

async function addLater(a, b) {
    let x = await Promise.resolve(a);
    let y = await Promise.resolve(b);
    return x + y;
}

async function seq(n) {
    let s = 0;
    for (let i = 0; i < n; i++) {
        s += await Promise.resolve(i);
    }
    return s;
}

function run() {
    return addLater(1, 2).then((v) => {
        return seq(3).then((w) => v + w);
    });
}

class Worker {
    async work(n) {
        return await seq(n);
    }
}

let w = new Worker();
run().then((v) => {
    w.work(2).then((u) => {
        console.log(v, u);
    });
});
