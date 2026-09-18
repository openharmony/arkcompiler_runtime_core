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

function fillMap() {
    let m = new Map();
    m.set('a', 1);
    m.set('b', 2);
    m.set('a', 3);
    return m.get('a') + m.size;
}

function fillSet() {
    let s = new Set([1, 2, 2, 3]);
    s.add(4);
    s.delete(2);
    return s.has(1) ? s.size : 0;
}

function fillWeak() {
    let key = {};
    let wm = new WeakMap();
    wm.set(key, 9);
    let ws = new WeakSet();
    ws.add(key);
    return wm.get(key) + (ws.has(key) ? 1 : 0);
}

function collect() {
    let m = new Map([['x', 1], ['y', 2]]);
    let s = 0;
    m.forEach((v, k) => {
        s += v + k.length;
    });
    return s;
}

console.log(fillMap(), fillSet(), fillWeak(), collect());
