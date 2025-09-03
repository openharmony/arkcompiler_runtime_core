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

namespace X {
    export class A {
        x: int

    }

    export interface B {

    }

    export class C implements B {
        y: String
    }

    export enum D {
        aa,
        bb
    }

    export function voidFunc(): void {}

    export function primFunc(): int {
        return 42
    }

    export function faFunc(): FixedArray<int> {
        let x: FixedArray<int> = [1, 2, 3]
        return x
    }

    export function objectFunc(): A {
        return new A()
    }

    export class funcContainer {
        public voidFunc(): void {}

        public primFunc(): int {
            return 42
        }

        public faFunc(): FixedArray<int> {
            let x: FixedArray<int> = [1, 2, 3]
            return x
        }

        public objectFunc(): A {
            return new A()
        }
    }

    interface E {

    }

    class F implements E {

    }

    export function returnHiddenClassObject(): F {
        return new F()
    }

    enum G {
        aa,
        bb
    }

    export function returnHiddenEnumValue(): G {
        return G.aa
    }
}

export class A {

}

export interface B {

}

export class C implements B {

}

export enum D {
    aa,
    bb
}


export function voidFunc(): void {}

export function primFunc(): int {
    return 42
}

export function faFunc(): FixedArray<int> {
    let x: FixedArray<int> = [1, 2, 3]
    return x
}

export function objectFunc(): A {
    return new A()
}

export class funcContainer {
    public voidFunc(): void {}

    public primFunc(): int {
        return 42
    }

    public faFunc(): FixedArray<int> {
        let x: FixedArray<int> = [1, 2, 3]
        return x
    }

    public objectFunc(): A {
        return new A()
    }
}

function checkClassIsNotEnumAndNotInterface() {
    for (let obj of [new X.C(), new X.A(), new C(), new A(), X.returnHiddenClassObject()]) {
        arktest.assertFalse(Class.of(obj).isEnum())
        arktest.assertFalse(Class.of(obj).isInterface())
    }
}

function checkEnumIsEnumAndNotInterface() {
    for (let obj of [X.D.aa, D.aa]) {
        arktest.assertTrue(Class.of(obj).isEnum())
        arktest.assertFalse(Class.of(obj).isInterface())
    }
    arktest.assertTrue(Class.of(X.returnHiddenEnumValue()).isEnum())
    arktest.assertFalse(Class.of(X.returnHiddenEnumValue()).isInterface())
}

function checkInterfaceIsInterfaceAndNotEnum() {
    let cls1 = Class.of(new X.C())
    let inf1 = cls1.getInterfaces()[0]
    let cls2 = Class.of(new C())
    let inf2 = cls2.getInterfaces()[0]
    let cls3 = Class.of(X.returnHiddenClassObject())
    let inf3 = cls3.getInterfaces()[0]
    for (let inf of [inf1, inf2, inf3]) {
        arktest.assertFalse(inf.isEnum())
        arktest.assertTrue(inf.isInterface())
    }
}

function checkFixedArray() {
    let varInt = 5
    let varDouble = 5.5
    let varPrimitiveArray: Array<int> = [1, 2, 3]
    let varObjectArray: Array<A> = [new A()]
    let varPrimitiveFixedArray: FixedArray<int> = [1, 2, 3]
    let varObjectFixedArray: FixedArray<A> = [new A()]
    let varObject = new A()
    let varString = "abc"
    arktest.assertFalse(Class.of(varInt).isFixedArray())
    arktest.assertFalse(Class.of(varDouble).isFixedArray())
    arktest.assertFalse(Class.of(varPrimitiveArray).isFixedArray())
    arktest.assertFalse(Class.of(varObjectArray).isFixedArray())
    arktest.assertTrue(Class.of(varPrimitiveFixedArray).isFixedArray())
    arktest.assertTrue(Class.of(varObjectFixedArray).isFixedArray())
    arktest.assertFalse(Class.of(varString).isFixedArray())
    arktest.assertFalse(Class.of(varObject).isFixedArray())
    arktest.assertFalse(Class.of(D.aa).isFixedArray())
    arktest.assertFalse(Class.of(new C()).getInterfaces()[0].isFixedArray())
    arktest.assertFalse(Class.of(faFunc).isFixedArray())
    arktest.assertFalse(Class.of(new funcContainer().faFunc).isFixedArray())
}

function checkFunctions() {
    let gc = new funcContainer()
    let nc = new X.funcContainer()
    let gList = [voidFunc, primFunc, faFunc, objectFunc]
    let gcList = [gc.voidFunc, gc.primFunc, gc.faFunc, gc.objectFunc]
    let nList = [X.voidFunc, X.primFunc, X.faFunc, X.objectFunc]
    let ncList = [nc.voidFunc, nc.primFunc, nc.faFunc, nc.objectFunc]
    for (let funcList of [gList, gcList, nList, ncList]) {
        for (let func of funcList) {
            let cls = Class.of(func)
            arktest.assertFalse(cls.isEnum())
            arktest.assertFalse(cls.isInterface())
            arktest.assertFalse(cls.isFixedArray())
        }
    }
}

function main(): void {
    let ts = new arktest.ArkTestsuite("Class API test")
    ts.addTest("checkClassIsNotEnumAndNotInterface", checkClassIsNotEnumAndNotInterface)
    ts.addTest("checkEnumIsEnumAndNotInterface", checkEnumIsEnumAndNotInterface)
    ts.addTest("checkInterfaceIsInterfaceAndNotEnum", checkInterfaceIsInterfaceAndNotEnum)
    ts.addTest("checkFixedArray", checkFixedArray)
    ts.addTest("checkFunctions", checkFunctions)
    ts.run();
}
