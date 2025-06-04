/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

function main(): int {
    const suite = new arktest.ArkTestsuite("deepcopy API tests for SDK");
    suite.addTest("Test DeepCopy Date", testdeepcopydate);
    suite.addTest("Test DeepCopy Map", testdeepcopymap);
    suite.addTest("Test DeepCopy Set", testdeepcopyset);
    suite.addTest("Test DeepCopy Array", testdeepcopyarray);
    suite.addTest("Test DeepCopy Enum", testdeepcopyenum);
    suite.addTest("Test DeepCopy Extends", testdeepcopyextends);
    return suite.run(); 
}

enum MyColor {
    White = 0
}

class A implements Cloneable {
    public math : number;
    constructor(math: number) {
        this.math = math;
    }

    clone() : A {
        let instance = new A(99);
        instance.math = this.math;
        return instance;
    }
}

class B implements Cloneable {
    public english : number;
    constructor(english: number) {
        this.english = english;
    }

    clone() : B {
        let instance = new B(99);
        instance.english = this.english;
        return instance;
    }
}

class C extends A {
    public physics : number;
    constructor(math: number, physics : number) {
        super(math);
        this.math = math;
        this.physics = physics;
    }

    clone() : C {
        let instance = new C(99, 99);
        instance.math = this.math;
        instance.physics = this.physics;
        return instance;
    }
}

class D extends A {
    public physics : number;
    constructor(math : number, physics : number) {
        super(math);
        this.math = math;
        this.physics = physics;
    }

    clone() : D {
        let instance = new D(99, 99);
        instance.math = this.math;
        return instance;
    }
}

class TestDate {
    static testDate = new Date(Date.UTC(1996, 1, 2, 3, 4, 5));
    static copiedDate = deepcopy(TestDate.testDate) as Date;
    static readonly expected = "Fri, 02 Feb 1996 07:30:05 GMT";
    static readonly copiedexpected = "Fri, 02 Feb 1996 03:04:45 GMT";
}

class TestMap {
    static testMap1 = new Map<number, string>([[1, 'a'], [2, 'A']]);
    static testMap2 = new Map<number, string>([[1, 'b'], [2, 'B']]);
    static testMap3 = new Map<number, string>([[1, 'c'], [2, 'C']]);
    static testMap = new Map<number, string>([
        [1, 'a'], 
        [2, 'b'],
        [3, 'c']
    ]);
    static testMap4 = new Map<number, Map<number, string>>([
        [1, TestMap.testMap1],
        [2, TestMap.testMap2],
        [3, TestMap.testMap3],
    ]);
    static copiedMap = deepcopy(TestMap.testMap) as Map<number, string>;
    static copiedMap1 = deepcopy(TestMap.testMap4) as Map<number, Map<number, string>>;
    static readonly expected = new Map<number, string>([
        [1, 'A'], 
        [2, 'b'],
        [3, 'c'],
        [4, 'd']
    ]);
    static readonly copiedexpected = new Map<number, string>([
        [1, 'a'], 
        [2, 'b'],
        [3, 'c']
    ]);
    static readonly testMap5 = new Map<number, string>([[1, 'A'], [2, 'C']]);
    static readonly expected1 = new Map<number, Map<number, string>>([
        [1, TestMap.testMap1],
        [2, TestMap.testMap2],
        [3, TestMap.testMap5],
    ]);
    static testMap6 = new Map<number, string>([[1, 'c'], [2, 'C']]);
    static readonly copiedexpected1 = new Map<number, Map<number, string>>([
        [1, TestMap.testMap1],
        [2, TestMap.testMap2],
        [3, TestMap.testMap6],
    ]);
}

class TestSet {
    static testSet = new Set<number>([1, 2, 3, 4]);
    static copiedSet = deepcopy(TestSet.testSet) as Set<number>;

    static classa = new A(110);
    static classb = new B(111);
    static testSet1 = new Set<A|B>([TestSet.classa, TestSet.classb]);
    static copiedSet1 = deepcopy(TestSet.testSet1) as Set<A|B>;

    static readonly expected = new Set<number>([1, 2, 3, 4, 5]);
    static readonly copiedexpected = new Set<number>([1, 2, 3, 4]);
    static readonly expected_math = 120;
    static readonly expected_english = 130;
    static readonly copiedexpected_math = 110;
    static readonly copiedexpected_english = 111;
}

class TestArray {
    static testArray = [1, 2, 3, 4];
    static copiedArray = deepcopy(TestArray.testArray) as Array<number>;
    static readonly expected = [1, 2, 3, 4, 5];
    static readonly copiedexpected = [1, 2, 3, 4];
}

class TestEnum {
    static mycolor = MyColor.White;
    static copiedcolor = deepcopy(TestEnum.mycolor) as MyColor;
    static readonly expected = MyColor.White;
}

class TestExtends {
    static classc = new C(100, 100);
    static cloneclassc = TestExtends.classc.clone() as C;
    static deepcopyclassc = deepcopy(TestExtends.classc) as C;

    static readonly expectedmath = 100;
    static readonly expectedphysics = 111;
    static readonly copiedexpectedmath = 100;
    static readonly copiedexpectedphysics = 100;

    static classd = new D(100, 100);
    static cloneclassd = TestExtends.classd.clone() as D;
    static deepcopyclassd = deepcopy(TestExtends.classd) as D; 

    static readonly expectedmath1 = 122;
    static readonly expectedphysics1 = 100;
    static readonly copiedexpectedmath1 = 100;
    static readonly copiedexpectedphysics1 = 99;
}

function testdeepcopydate() {
    TestDate.testDate.setUTCHours(7);
    TestDate.testDate.setUTCMinutes(30);
    TestDate.copiedDate.setUTCSeconds(45);
    assertEQ(TestDate.expected, TestDate.testDate.toUTCString(), `Expected: "${TestDate.expected}", Actual: "${TestDate.testDate.toUTCString()}"`);
    assertEQ(TestDate.copiedexpected, TestDate.copiedDate.toUTCString(), `Expected: "${TestDate.copiedexpected}", Actual: "${TestDate.copiedDate.toUTCString()}"`);
}

function testdeepcopymap() {
    TestMap.testMap.set(1, 'A');
    TestMap.testMap.set(4, 'd');

    TestMap.testMap3.set(1, 'A');

    for (let i = 1; i <= TestMap.testMap.size; i++) {
        assertEQ(TestMap.expected.get(i), TestMap.testMap.get(i), `Expected: "${TestMap.expected.get(i)}", Actual: "${TestMap.testMap.get(i)}"`);
    }

    for (let j = 1; j <= TestMap.copiedMap.size; j++) {
        assertEQ(TestMap.copiedexpected.get(j), TestMap.copiedMap.get(j), `Expected: "${TestMap.copiedexpected.get(j)}", Actual: "${TestMap.copiedMap.get(j)}"`);
    }

    for (let m = 1; m <= TestMap.testMap4.size; m++) {
        let temp = TestMap.testMap4.get(m) as Map<number, string>;
        let temp1 = TestMap.expected1.get(m) as Map<number, string>;
        for (let n = 1; n <= temp.size; n++) {
            assertEQ(temp1.get(n), temp.get(n), `Expected: "${temp1.get(n)}", Actual: "${temp.get(n)}"`);
        }
    }

    for (let p = 1; p <= TestMap.copiedMap1.size; p++) {
        let temp = TestMap.copiedMap1.get(p) as Map<number, string>;
        let temp1 = TestMap.copiedexpected1.get(p) as Map<number, string>;
        for (let q = 1; q <= temp.size; q++) {
            assertEQ(temp1.get(q), temp.get(q), `Expected: "${temp1.get(q)}", Actual: "${temp.get(q)}"`);
        }
    }
}

function testdeepcopyset() {
    TestSet.testSet.add(5);
    TestSet.classa.math = 120;
    TestSet.classb.english = 130;

    for (const item of TestSet.testSet1) {
        if (item instanceof A) {
            assertEQ(TestSet.expected_math, item.math, `Expected: "${TestSet.expected_math}", Actual: "${item.math}"`);
        } else if (item instanceof B) {
            assertEQ(TestSet.expected_english, item.english, `Expected: "${TestSet.expected_english}", Actual: "${item.english}"`);
        }
    }

    for (const item of TestSet.copiedSet1) {
        if (item instanceof A) {
            assertEQ(TestSet.copiedexpected_math, item.math, `Expected: "${TestSet.copiedexpected_math}", Actual: "${item.math}"`);
        } else if (item instanceof B) {
            assertEQ(TestSet.copiedexpected_english, item.english, `Expected: "${TestSet.copiedexpected_english}", Actual: "${item.english}"`);
        }
    }

    if (TestSet.testSet.size == TestSet.expected.size) {
        for (let item of TestSet.testSet) {
            if (!TestSet.expected.has(item)) {
                assertEQ(TestSet.expected, TestSet.testSet, `Expected: "${TestSet.expected}", Actual: "${TestSet.testSet}"`);
            }
        }
    } else {
        assertEQ(TestSet.expected, TestSet.testSet, `Expected: "${TestSet.expected}", Actual: "${TestSet.testSet}"`);
    }

    if (TestSet.copiedSet.size == TestSet.copiedexpected.size) {
        for (let item of TestSet.copiedSet) {
            if (!TestSet.copiedexpected.has(item)) {
                assertEQ(TestSet.copiedexpected, TestSet.copiedSet, `Expected: "${TestSet.copiedexpected}", Actual: "${TestSet.copiedSet}"`);
            }
        }
    } else {
        assertEQ(TestSet.copiedexpected, TestSet.copiedSet, `Expected: "${TestSet.copiedexpected}", Actual: "${TestSet.copiedSet}"`);
    }
}

function testdeepcopyarray() {
    TestArray.testArray.push(5);

    for (let i = 0; i < TestArray.testArray.length; i++) {
        assertEQ(TestArray.expected[i], TestArray.testArray[i], `Expected: "${TestArray.expected[i]}", Actual: "${TestArray.testArray[i]}"`);
    }

    for (let j = 0; j < TestArray.copiedArray.length; j++) {
        assertEQ(TestArray.copiedexpected[j], TestArray.copiedArray[j], `Expected: "${TestArray.copiedexpected[j]}", Actual: "${TestArray.copiedArray[j]}"`);
    }

    TestArray.testArray.splice(4,1);
}

function testdeepcopyenum() {
    assertEQ(TestEnum.expected, TestEnum.mycolor, `Expected: "${TestEnum.expected}", Actual: "${TestEnum.mycolor}"`);
    assertEQ(TestEnum.expected, TestEnum.copiedcolor, `Expected: "${TestEnum.expected}", Actual: "${TestEnum.copiedcolor}"`);
}

function testdeepcopyextends() {
    TestExtends.classc.physics = 111;
    TestExtends.classd.math = 122;

    assertEQ(TestExtends.expectedmath, TestExtends.classc.math, `Expected: "${TestExtends.expectedmath}", Actual: "${TestExtends.classc.math}"`);
    assertEQ(TestExtends.expectedphysics, TestExtends.classc.physics, `Expected: "${TestExtends.expectedphysics}", Actual: "${TestExtends.classc.physics}"`);
    assertEQ(TestExtends.copiedexpectedmath, TestExtends.cloneclassc.math, `Expected: "${TestExtends.copiedexpectedmath}", Actual: "${TestExtends.cloneclassc.math}"`);
    assertEQ(TestExtends.copiedexpectedphysics, TestExtends.cloneclassc.physics, `Expected: "${TestExtends.copiedexpectedphysics}", Actual: "${TestExtends.cloneclassc.physics}"`);
    assertEQ(TestExtends.copiedexpectedmath, TestExtends.deepcopyclassc.math, `Expected: "${TestExtends.copiedexpectedmath}", Actual: "${TestExtends.deepcopyclassc.math}"`);
    assertEQ(TestExtends.copiedexpectedphysics, TestExtends.deepcopyclassc.physics, `Expected: "${TestExtends.copiedexpectedphysics}", Actual: "${TestExtends.deepcopyclassc.physics}"`);
    
    assertEQ(TestExtends.expectedmath1, TestExtends.classd.math, `Expected: "${TestExtends.expectedmath1}", Actual: "${TestExtends.classd.math}"`);
    assertEQ(TestExtends.expectedphysics1, TestExtends.classd.physics, `Expected: "${TestExtends.expectedphysics1}", Actual: "${TestExtends.classd.physics}"`);
    assertEQ(TestExtends.copiedexpectedmath1, TestExtends.cloneclassd.math, `Expected: "${TestExtends.copiedexpectedmath1}", Actual: "${TestExtends.cloneclassd.math}"`);
    assertEQ(TestExtends.copiedexpectedphysics1, TestExtends.cloneclassd.physics, `Expected: "${TestExtends.copiedexpectedphysics1}", Actual: "${TestExtends.cloneclassd.physics}"`);
    assertEQ(TestExtends.copiedexpectedmath1, TestExtends.deepcopyclassd.math, `Expected: "${TestExtends.copiedexpectedmath1}", Actual: "${TestExtends.deepcopyclassd.math}"`);
    assertEQ(TestExtends.copiedexpectedphysics1, TestExtends.deepcopyclassd.physics, `Expected: "${TestExtends.copiedexpectedphysics1}", Actual: "${TestExtends.deepcopyclassd.physics}"`);
}
