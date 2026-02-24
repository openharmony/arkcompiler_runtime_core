/*---
Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
  Try block and entire try statement complete normally if no catch block is
  executed. Try block completes abruptly if an error is thrown. If catch clause
  is present and catch body completes normally, the entire try statement completes
  normally. If no catch clause is in place, the error is propagated to surrounding
  and caller scopes. If finally clause is in place and its execution completes
  abruptly, then the try statement also completes abruptly.
---*/

class Err1 extends Error {}
class Err2 extends Error {}

interface I {}
class ErrI extends Error implements I {}

function innerThrower(): void {
    throw new Err1()
}

function middleCaller(): void {
    innerThrower()
}

function recurse(n: int): int {
    if (n <= 0) throw new Err1()
    return recurse(n - 1)
}

class ErrRec extends Error {}

function fooRecursive(e: ErrRec, cnt: int): ErrRec {
    if (cnt > 0) throw fooRecursive(e, --cnt);
    return e
}

function main(): int {
    let suite = new arktest.ArkTestsuite("try Statement Execution")

    suite.addTest("Error propagation to caller context via recursive throw", () => {
        try {
            fooRecursive(new ErrRec(), 100);
        } catch (e) {
            arktest.assertTrue(e instanceof ErrRec)
        }
    })

    suite.addTest("Assignment compatible to error with instanceof", () => {
        let err: I = new ErrI();
        try {
            if (1 != 2) throw (err as Object) as Error;
        } catch (e) {
            arktest.assertTrue(e instanceof I)
            arktest.assertTrue(e instanceof ErrI)
        }
    })

    suite.addTest("Rule 1: Statements after throw in try block are not executed", () => {
        let result = ""
        try {
            result += "A"
            throw new Err1()
        } catch (e) {
            result += "B"
        }
        arktest.assertEQ(result, "AB")
    })

    suite.addTest("Rule 2: Catch body completes normally, try statement completes normally", () => {
        let afterTry = false
        try {
            throw new Err1()
        } catch (e) {
            let handled = true
        }
        afterTry = true
        arktest.assertTrue(afterTry)
    })

    suite.addTest("Rule 3: Error propagates through multiple caller scopes", () => {
        let caught = false
        try {
            middleCaller()
        } catch (e) {
            caught = e instanceof Err1
        }
        arktest.assertTrue(caught)
    })

    suite.addTest("Rule 3: Error propagates through recursive calls", () => {
        let caught = false
        try {
            recurse(5)
        } catch (e) {
            caught = e instanceof Err1
        }
        arktest.assertTrue(caught)
    })

    suite.addTest("Rule 4: Finally completes abruptly, try statement completes abruptly", () => {
        let outerCaught = false
        try {
            try {
                let x = 10
            } finally {
                throw new Err1()
            }
        } catch (e) {
            outerCaught = e instanceof Err1
        }
        arktest.assertTrue(outerCaught)
    })

    suite.addTest("Rule 4: Finally abrupt completion overrides try normal completion", () => {
        let result = ""
        try {
            try {
                result += "try"
            } finally {
                result += "-finally"
                throw new Err1()
            }
        } catch (e) {
            result += "-caught"
        }
        arktest.assertEQ(result, "try-finally-caught")
    })

    suite.addTest("Combined: try-catch-finally normal flow", () => {
        let result = ""
        try {
            result += "1"
        } catch (e) {
            result += "2"
        } finally {
            result += "3"
        }
        arktest.assertEQ(result, "13")
    })

    suite.addTest("Combined: try-catch-finally with error", () => {
        let result = ""
        try {
            result += "1"
            throw new Err1()
        } catch (e) {
            result += "2"
        } finally {
            result += "3"
        }
        arktest.assertEQ(result, "123")
    })

    suite.addTest("Error propagation through deeply nested try blocks", () => {
        let result = ""
        try {
            try {
                try {
                    throw new Err1()
                } finally {
                    result += "A"
                }
            } finally {
                result += "B"
            }
        } catch (e) {
            result += "C"
        }
        arktest.assertEQ(result, "ABC")
    })

    return suite.run()
}
