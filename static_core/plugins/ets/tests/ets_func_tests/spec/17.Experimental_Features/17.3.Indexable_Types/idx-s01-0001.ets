/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

/*---
 desc: 17.3 Indexable Types
 tags:
 ---*/


class Bar {
    private data : int;
    private data2: int;
    private data3: int;

    $_get (index: int) : int throws {
        switch(index) {
        case 0:
            return this.data;
        case 1:
            return this.data2;
        case 2:
            return this.data3;
        default:
            throw new Exception ("Hmm ...");
        }
    }

    $_set (index: int, value: int)  throws {
        switch(index) {
        case 0:
            this.data = value;
            break;
        case 1:
            this.data2 = value;
            break;
        case 2:
            this.data3 = value;
            break;
        default:
            throw new Exception ("Hmm ...");
        }
    }

    constructor(data : int, data2: int, data3: int) {
        this.data = data;
        this.data2 = data2;
        this.data3 = data3;
    }
}

function main(): int {
    let x : Bar = new Bar(1,2,3);
    try {
        assertEQ(x.$_get(0), 1)
        assertEQ(x.$_get(1), 2)
        assertTrue((x.$_get(2)== 3))
    } catch (e) {
        console.log("1");
        return 1;
    }

    try {
        x.$_set(0, 4);
        x.$_set(1, 5);
        x.$_set(2, 6);
    } catch (e) {
        console.log("2")
        return 1;
    }

    try {
        assertEQ(x.$_get(0), 4)
        assertEQ(x.$_get(1), 5)
        assertEQ(x.$_get(2), 6)
    } catch (e) {
        console.log("3");
        return 1;
    }

    return 0;
}
