/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

function indexOfTest1() {
    let a: number[] = [1, 2, 3];
    arktest.assertEQ(a.indexOf(2), 1);
    arktest.assertEQ(a.indexOf(4), -1);
}

function indexOfTest2() {
    let a: string[] = ['a', 'b', 'c'];
    arktest.assertEQ(a.indexOf('b'), 1);
    arktest.assertEQ(a.indexOf('d'), -1);
}

function indexOfTest3() {
    let one = 1;
    let a: Array<Object | string | number> = new Array<Object | string | number>(one, 1);
    arktest.assertEQ(a.indexOf(1), 0);
}

function indexOfTest4() {
    let one = 1;
    let two = 2;
    let three = 3;
    let a: Array<Object | string | number> = new Array<Object | string | number>(
        'start', one, 1, 'middle', two, 2, 'end', three, 3, {}
    );
    arktest.assertEQ(a.indexOf(1), 1);
    arktest.assertEQ(a.indexOf(2), 4);
    arktest.assertEQ(a.indexOf(3), 7);
    
}
function indexOfTest5() {
    let val10: byte = 10;
    let val20: short = 20;
    let val30: long = 30;
    let val40: float = 40.0;
    let a: Array<Object | string | number | boolean> = 
    new Array<Object | string | number | boolean>(true, false, val10, 'x', 10.0, 'y', val20, 'z', 20.0, val30, val40, 30, 40.0);

    arktest.assertEQ(a.indexOf(10), 2);
    arktest.assertEQ(a.indexOf(20), 6);
    arktest.assertEQ(a.indexOf(30), 9);
    arktest.assertEQ(a.indexOf(40.0), 10);
}

function lastIndexOfTest1() {
    let a: number[] = [1, 2, 3, 2, 1];
    arktest.assertEQ(a.lastIndexOf(2), 3);
    arktest.assertEQ(a.lastIndexOf(4), -1);
}

function lastIndexOfTest2() {
    let a: string[] = ['a', 'b', 'c', 'b', 'a'];
    arktest.assertEQ(a.lastIndexOf('b'), 3);
    arktest.assertEQ(a.lastIndexOf('d'), -1);
}

function lastIndexOfTest3() {
    let one = 1;
    let a: Array<Object | string | number> = new Array<Object | string | number>(1, one);
    arktest.assertEQ(a.lastIndexOf(1), 1);
}

function lastIndexOfTest4() {
    let one = 1;
    let two = 2;
    let three = 3;
    let a: Array<Object | string | number> = new Array<Object | string | number>(
        'start', one, 1, 'middle', two, 2, 'end', three, 3, {}
    );
    arktest.assertEQ(a.lastIndexOf(1), 2);
    arktest.assertEQ(a.lastIndexOf(2), 5);
    arktest.assertEQ(a.lastIndexOf(3), 8);
}

function lastIndexOfTest5() {
    let val10: byte = 10;
    let val20: short = 20;
    let val30: long = 30;
    let val40: float = 40.0;
    let a: Array<Object | string | number | boolean> = 
    new Array<Object | string | number | boolean>(true, false, val10, 'x', 10.0, 'y', val20, 'z', 20.0, val30, val40, 30, 40.0);

    arktest.assertEQ(a.lastIndexOf(10), 4);
    arktest.assertEQ(a.lastIndexOf(20), 8);
    arktest.assertEQ(a.lastIndexOf(30), 11);
    arktest.assertEQ(a.lastIndexOf(40.0), 12);
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Array indexof tests');
    suite.addTest('indexof test 1', indexOfTest1);
    suite.addTest('indexof test 2', indexOfTest2);
    suite.addTest('indexof test 3', indexOfTest3);
    suite.addTest('indexof test 4', indexOfTest4);
    suite.addTest('indexof test 5', indexOfTest5);
    suite.addTest('lastindexof test 1', lastIndexOfTest1);
    suite.addTest('lastindexof test 2', lastIndexOfTest2);
    suite.addTest('lastindexof test 3', lastIndexOfTest3);
    suite.addTest('lastindexof test 4', lastIndexOfTest4);
    suite.addTest('lastindexof test 5', lastIndexOfTest5);
    return suite.run();
}
