/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

class Bar  {
    private data1 : int;

    $_get (index: int) : int throws {
        switch(index) {
        case 0:
            return this.data1;
        default:
            throw new Exception ("Hmm ...");
        }
    }

    $_set (index: int, value: int)  throws {
        switch(index) {
        case 0:
            this.data1 = value;
            break;
        default:
            throw new Exception ("Hmm ...");
        }
    }

    $_get (index: string) : int throws {
        switch(index) {
        case "hello":
            return this.data1;
        default:
            throw new Exception ("Hmm ...");
        }
    }

    $_set (index: string, value: int)  throws {
        switch(index) {
        case "hello":
            this.data1 = value;
            break;
        default:
            throw new Exception ("Hmm ...");
        }
    }

    constructor(data : int) {
        this.data1 = data;
    }
}



function test1() : int {
    let result = 0;
    let x = new Bar(10);

    try {
        assert(x[0] == 10)
    } catch(e) {
        result++
        console.log("Unexpexted exception 1");
    }

    try {
        assert(x["hello"] == 10)
    } catch(e) {
        result++
        console.log("Unexpexted exception 1");
    }

    try {
        x[0] = 100
    } catch(e) {
        result++
        console.log("Unexpexted exception 3");
    }

    try {
        assert(x[0] == 100)
    } catch(e) {
        result++
        console.log("Unexpexted exception 4");
    }

    try {
        x["hello"] = 100
    } catch(e) {
        result++
        console.log("Unexpexted exception 5");
    }

    try {
        assert(x["hello"] == 100)
    } catch(e) {
        result++
        console.log("Unexpexted exception 6");
    }

    return result;
}

function test2() : int {
    let result = 0;
    let x = new Bar(10);
    try {
        assert(x[10] == 1)
    } catch(e) {
        result++
    }

    try {
        assert(x["bye"] == 1)
    } catch(e) {
        result++
    }


    return result;
}

function main() {
    assert(test1() == 0);
    assert(test2() == 2);
}

