/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

class Connection {
    public id: number;
    public user: string;

    constructor(id: number, user: string) {
        this.id = id;
        this.user = user;
    }
}

function main() {
    let d: Connection[] = [
        new Connection(1, 'user1'),
        new Connection(2, 'user2')
    ]

    // Debug arrays
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.debug('Active connections:', d);
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.debug([1, 2, 3, 4, 5]);

    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.info('Active connections:', d);
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.info([1, 2, 3, 4, 5]);

    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log('Active connections:', d);
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log([1, 2, 3, 4, 5]);

    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.warn('Active connections:', d);
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.warn([1, 2, 3, 4, 5]);

    // Note(ivan-tyulyandin): uncomment and fix .expected file after fix of #25014
    // FIXME console.error("Active connections:", d);
    // FIXME console.error([1, 2, 3, 4, 5]);
}
