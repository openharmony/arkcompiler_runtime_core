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

class BarInt1  {
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

    constructor(data : int) {
        this.data1 = data;
    }
}


class BarInt2 extends BarInt1 {
    private data2 : int;
    private data21: int;

    override $_get (index: int) : int throws {
        switch(index) {
        case 0:
            return this.data2;
        case 1:
            return this.data21;
        default:
            throw new Exception ("Hmm ...");
        }
    }

    override $_set (index: int, value: int)  throws {
        switch(index) {
        case 0:
            this.data2 = value;
            break;
        case 1:
            this.data21 = value;
            break;
        default:
            throw new Exception ("Hmm ...");
        }
    }

    constructor(data : int, data2: int) {
        super(data);
        this.data2 = data;
        this.data21 = data2;
    }
}

function createTestObject1() : BarInt1 {
    return new BarInt1(1);
}

function createTestObject2() : BarInt1 {
    return new BarInt2(1,2);
}

function test1() : int {
    let result = 0;
    let x = createTestObject1();
    let y = createTestObject2();
    try {
        assertEQ(x[0], 1)
    } catch(e) {
        result++
        console.log("Unexpexted exception 1");
    }

    try {
        assertEQ(y[0], 1)
        assertEQ(y[1], 2)
    } catch(e) {
        result++
        console.log("Unexpexted exception 2");

    }

    try {
        x[0] = 10
    } catch(e) {
        result++
        console.log("Unexpexted exception 3");
    }

    try {
        y[0] = 10
        y[1] = 20
    } catch(e) {
        result++
        console.log("Unexpexted exception 4");
    }

      try {
        assertEQ(x[0], 10)
    } catch(e) {
        result++
        console.log("Unexpexted exception 5");
    }

    try {
        assertEQ(y[0], 10)
        assertEQ(y[1], 20)
    } catch(e) {
        result++
        console.log("Unexpexted exception 6");

    }


    return result;
}

function test2() : int {
    let result = 0;
    let x = createTestObject1();
    let y = createTestObject2();
    try {
        assertEQ(x[10], 1)
    } catch(e) {
        result++
    }

    try {
        assertEQ(y[110], 1)
        assertEQ(y[120], 2)
    } catch(e) {
        result++
    }

    return result;
}

function main() {
    assertEQ(test1(), 0)
    assertEQ(test2(), 2)
}

