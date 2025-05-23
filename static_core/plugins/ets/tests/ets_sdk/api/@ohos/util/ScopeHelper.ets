/**
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

import {util} from "@ohos.util"

class NumberType implements util.ScopeType<NumberType> {
    n: number
    constructor(n: number) {
      this.n = n
    }
    
    // NOTE (aleksander-sotov): Remove after internal issue #22930 fix
    compareTo(other: Object): boolean {
       return this.compareTo(other as NumberType)
    }
   
    compareTo(other: NumberType): boolean {
       	return this.n >= other.n
    }
   
    toString(): string {
        return `${this.n}`
    }
}

function createScopeHelper(a: number, b: number): util.ScopeHelper<NumberType> {
    let a1 = new NumberType(a)
    let a2 = new NumberType(b)
    return new util.ScopeHelper<NumberType>(a1, a2)
}

function main() {
    testIntersectByTwoValues()
    testIntersectByRange()
    testIntersectWrongRanges()
    testExpandWithTwoValues()
    testExpandWithOneValue()
    testExpandWithRange()
    testContainsWithValue()
    testContainsWithRange()
    testClamp()
    testToString()
    testLowerUpperAreEqual()
}

function testIntersectByTwoValues() {
    let sh = createScopeHelper(10, 30)

    let res = sh.intersect(new NumberType(15), new NumberType(30))
    assertEQ(res.getLower().n, 15)
    assertEQ(res.getUpper().n, 30)

    res = sh.intersect(new NumberType(15), new NumberType(20))
    assertEQ(res.getLower().n, 15)
    assertEQ(res.getUpper().n, 20)

    res = sh.intersect(new NumberType(10), new NumberType(30))
    assertEQ(res.getLower().n, 10)
    assertEQ(res.getUpper().n, 30)

    res = sh.intersect(new NumberType(5), new NumberType(40))
    assertEQ(res.getLower().n, 10)
    assertEQ(res.getUpper().n, 30)
}

function testIntersectByRange() {
    let sh = createScopeHelper(10, 30)

    let other = createScopeHelper(1, 11)
    assertEQ(sh.intersect(other).getLower().n, 10)
    assertEQ(sh.intersect(other).getUpper().n, 11)

    other = createScopeHelper(10, 20)
    assertEQ(sh.intersect(other).getLower().n, 10)
    assertEQ(sh.intersect(other).getUpper().n, 20)

    other = createScopeHelper(20, 300)
    assertEQ(sh.intersect(other).getLower().n, 20)
    assertEQ(sh.intersect(other).getUpper().n, 30)

    other = createScopeHelper(0, 300)
    assertEQ(sh.intersect(other).getLower().n, 10)
    assertEQ(sh.intersect(other).getUpper().n, 30)
}

function testIntersectWrongRanges() {
    let sh = createScopeHelper(10, 30)
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof Error) && ((e as Error).toString() == "Error: lower limit must be less than upper limit")
    }
    expectThrow(() => {sh.intersect(createScopeHelper(100, 200))}, exceptionCheck)
    expectThrow(() => {sh.intersect(createScopeHelper(0, 2))}, exceptionCheck)
}

function testLowerUpperAreEqual() {
    
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof Error) && ((e as Error).toString() == "Error: lower limit must be less than upper limit")
    }
    expectThrow(() => {createScopeHelper(10, 10)}, exceptionCheck)
    
}

function testExpandWithTwoValues() {
    let sh = createScopeHelper(10, 30)

    let res = sh.expand(new NumberType(15), new NumberType(30))
    assertEQ(res.getLower().n, 10)
    assertEQ(res.getUpper().n, 30)

    res = sh.expand(new NumberType(15), new NumberType(20))
    assertEQ(res.getLower().n, 10)
    assertEQ(res.getUpper().n, 30)

    res = sh.expand(new NumberType(10), new NumberType(30))
    assertEQ(res.getLower().n, 10)
    assertEQ(res.getUpper().n, 30)

    res = sh.expand(new NumberType(5), new NumberType(40))
    assertEQ(res.getLower().n, 5)
    assertEQ(res.getUpper().n, 40)
}

function testExpandWithOneValue() {
    let sh = createScopeHelper(10, 30)

    let res = sh.expand(new NumberType(5))
    assertEQ(res.getLower().n, 5)
    assertEQ(res.getUpper().n, 30)

    res = sh.expand(new NumberType(20))
    assertEQ(res.getLower().n, 10)
    assertEQ(res.getUpper().n, 30)

    res = sh.expand(new NumberType(30))
    assertEQ(res.getLower().n, 10)
    assertEQ(res.getUpper().n, 30)

    res = sh.expand(new NumberType(40))
    assertEQ(res.getLower().n, 10)
    assertEQ(res.getUpper().n, 40)
}

function testExpandWithRange() {
    let sh = createScopeHelper(10, 30)

    let res = sh.expand(createScopeHelper(15, 30))
    assertEQ(res.getLower().n, 10)
    assertEQ(res.getUpper().n, 30)

    res = sh.expand(createScopeHelper(15, 20))
    assertEQ(res.getLower().n, 10)
    assertEQ(res.getUpper().n, 30)

    res = sh.expand(createScopeHelper(10, 30))
    assertEQ(res.getLower().n, 10)
    assertEQ(res.getUpper().n, 30)

    res = sh.expand(createScopeHelper(5, 40))
    assertEQ(res.getLower().n, 5)
    assertEQ(res.getUpper().n, 40)
}

function testContainsWithValue() {
    let sh = createScopeHelper(10, 30)

    assertEQ(sh.contains(new NumberType(10)), true)
    assertEQ(sh.contains(new NumberType(23)), true)
    assertEQ(sh.contains(new NumberType(30)), true)
    assertEQ(sh.contains(new NumberType(40)), false)
    assertEQ(sh.contains(new NumberType(0)), false)
}

function testContainsWithRange() {
    let sh = createScopeHelper(10, 30)

    assertEQ(sh.contains(createScopeHelper(10, 30)), true)
    assertEQ(sh.contains(createScopeHelper(10, 20)), true)
    assertEQ(sh.contains(createScopeHelper(15, 18)), true)
    assertEQ(sh.contains(createScopeHelper(10, 40)), false)
    assertEQ(sh.contains(createScopeHelper(1, 30)), false)
}

function testClamp() {
    let sh = createScopeHelper(10, 30)

    assertEQ(sh.clamp(new NumberType(5)).n, 10)
    assertEQ(sh.clamp(new NumberType(10)).n, 10)
    assertEQ(sh.clamp(new NumberType(23)).n, 23)
    assertEQ(sh.clamp(new NumberType(30)).n, 30)
    assertEQ(sh.clamp(new NumberType(1000)).n, 30)
}

function testToString() {
    let sh = createScopeHelper(10, 30)

    assertEQ(sh.toString(), "[10, 30]")
}
