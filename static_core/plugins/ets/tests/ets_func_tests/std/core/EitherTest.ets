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
 * WITHOUT WClassTwoRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

function testEither<T1, T2>(fst: T1, snd: T2) {
    let lFst: Either<T1, T2> = Left<T1, T2>(fst)
    arktest.assertEQ(lFst.isLeft(), true)
    arktest.assertEQ(lFst.isRight(), false)
    arktest.assertEQ(lFst.left, fst)
    arktest.expectError(() => {lFst.right}, new Error("Cannot read Right value from Left!"))

    let rSnd: Either<T1, T2> = Right<T1, T2>(snd)
    arktest.assertEQ(rSnd.isLeft(), false)
    arktest.assertEQ(rSnd.isRight(), true)
    arktest.assertEQ(rSnd.right, snd)
    arktest.expectError(() => {rSnd.left}, new Error("Cannot read Left value from Right!"))

    // Swap type positions
    let lSnd: Either<T2, T1> = Left<T2, T1>(snd)
    arktest.assertEQ(lSnd.isLeft(), true)
    arktest.assertEQ(lSnd.isRight(), false)
    arktest.assertEQ(lSnd.left, snd)
    arktest.expectError(() => {lSnd.right}, new Error("Cannot read Right value from Left!"))

    let rFst: Either<T2, T1> = Right<T2, T1>(fst)
    arktest.assertEQ(rFst.isLeft(), false)
    arktest.assertEQ(rFst.isRight(), true)
    arktest.assertEQ(rFst.right, fst)
    arktest.expectError(() => {rFst.left}, new Error("Cannot read Left value from Right!"))
}

class ClassOne {}
class ClassTwo {}
type TupleClassOne = [ClassOne, ClassOne]
type LambdaOne = () => void
type LambdaTwo = (a: ClassOne) => ClassTwo

function main() {
    const suite = new arktest.ArkTestsuite('Either tests')
    suite.addTest('classes', () => testEither<ClassOne, ClassTwo>(new ClassOne, new ClassTwo))
    suite.addTest('one undefined', () => testEither<ClassOne, undefined>(new ClassOne, undefined))
    suite.addTest('one null', () => testEither<ClassOne, null>(new ClassOne, null))
    suite.addTest('one tuple', () => testEither<TupleClassOne, ClassTwo>([new ClassOne, new ClassOne], new ClassTwo))
    suite.addTest('one lambda', () => testEither<ClassOne, LambdaOne>(new ClassOne, () => {}))
    suite.addTest('both undefined', () => testEither<undefined, undefined>(undefined, undefined))
    suite.addTest('both null', () => testEither<null, null>(null, null))
    suite.addTest('both tuple', () => testEither<TupleClassOne, TupleClassOne>([new ClassOne, new ClassOne], [new ClassOne, new ClassOne]))
    suite.addTest('two lambda', () => testEither<LambdaOne, LambdaTwo>(() => {}, (a: ClassOne) => new ClassTwo))
    let d = new ClassOne
    suite.addTest('same object', () => testEither<ClassOne, ClassOne>(d, d))
    let td: TupleClassOne = [new ClassOne, new ClassOne]
    suite.addTest('same tuple', () => testEither<TupleClassOne, TupleClassOne>(td, td))
    let ld: LambdaOne = () => {}
    suite.addTest('same lambda', () => testEither<LambdaOne, LambdaOne>(ld, ld))
    return suite.run()
}