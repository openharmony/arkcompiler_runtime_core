/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: >-
    In case of function types variance interleaving occurs at every nesting level. Depth-4 interleaving
    matches the spec example "and further more" continuation; also runtime verification of interleaved types.
---*/

class XClassDepth4<in T1, out T2> {
    foo(p: (q: (r: (s: (t: T1) => T2) => T1) => T2) => T1): T2 {
        return "" as T2
    }
}

interface IXIntfDepth4<in T1, out T2> {
    foo(p: (q: (r: (s: (t: T1) => T2) => T1) => T2) => T1): T2
}

class IXIntfDepth4Impl implements IXIntfDepth4<Object, String> {
    foo(p: (q: (r: (s: (t: Object) => String) => Object) => String) => Object): String {
        return ""
    }
}

class XRuntime<in T1, out T2> {
    readonly produced: T2
    constructor(produced: T2) { this.produced = produced }
    pass(p: T1): T2 { return this.produced }
    bar(callback: (q: T2) => T1): void {
        let r: T1 = callback(this.produced)
    }
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Variance interleaving depth-4 and runtime")

    suite.addTest("class declaration with depth-4 interleaving compiles", () => {
        let x = new XClassDepth4<Object, String>()
        arktest.assertEQ(x.foo((q): Object => new Object()), "")
    })

    suite.addTest("interface declaration with depth-4 interleaving compiles", () => {
        let i: IXIntfDepth4<Object, String> = new IXIntfDepth4Impl()
        arktest.assertEQ(i.foo((q): Object => new Object()), "")
    })

    suite.addTest("level-0 runtime: in T1, out T2 propagates value", () => {
        let x = new XRuntime<Object, String>("hello")
        arktest.assertEQ(x.pass(new Object()), "hello")
    })

    suite.addTest("level-1 runtime: callback (T2) => T1 invoked correctly", () => {
        let x = new XRuntime<Object, String>("payload")
        let received: String = ""
        x.bar((s: String): Object => {
            received = s
            return s
        })
        arktest.assertEQ(received, "payload")
    })

    return suite.run()
}
