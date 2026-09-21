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

function matchId(s) {
    let re = /^id-(\d+)$/;
    let m = re.exec(s);
    return m ? Number(m[1]) : -1;
}

function splitWords(s) {
    return s.split(/\s+/).filter((w) => w.length > 0);
}

function fmt(name, n) {
    return `${name}:${n.toFixed(1)}`;
}

function joinAll(parts) {
    return parts.map((p) => fmt(p, p.length)).join('|');
}

let ids = [matchId('id-12'), matchId('x')];
let words = splitWords('a  bb   ccc');
console.log(ids[0], ids[1], joinAll(words), /ab+c/.test('abbbc'));
