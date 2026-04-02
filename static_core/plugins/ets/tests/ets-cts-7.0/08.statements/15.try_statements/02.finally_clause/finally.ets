/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
desc: >-
  A finally clause defines the set of actions in the form of a block to be
  executed without regard to whether a try-catch completes normally or abruptly.
---*/

class Er extends Error {}
class Er2 extends Er {}

let t1 = 1
let t2 = 2
let ret: int = 0
let rftA1 = 0, rftA2 = 0, rftA3 = 0
let rntA1 = 0, rntA2 = 0, rntA3 = 0, rntA4 = 0

function fooNoSuitableCatch(): int {
    try {
        if (t1 != t2) throw new Er()
        ret += 1
    } finally {
        ret += 100
    }
    return ret;
}

function fooWithSuitableCatch(): int {
    try {
        if (t1 != t2) throw new Er2()
        ret += 1
    } catch (e) {
        ret += 10
    } finally {
        ret += 1000
    }
    return ret;
}

function fooRethrowFromCatch(): int {
    try {
        if (t1 != t2) throw new Er2()
        ret += 1
    } catch (e) {
        ret += 10
        throw e
    } finally {
        ret += 1000
    }
    return ret;
}

function fooReturnFromTry(): void {
    rftA1 = 0; rftA2 = 0; rftA3 = 0;
    try {
        rftA1 = 1;
        if (t1 != t2) {
            return;
        }
        rftA2 = 1;
    } finally {
        rftA3 = 1;
    }
}

function fooReturnFromNestedTry(): void {
    rntA1 = 0; rntA2 = 0; rntA3 = 0; rntA4 = 0;
    try {
        try {
            rntA1 = 1;
            if (t1 != t2) {
                return;
            }
            rntA2 = 1;
        } finally {
            rntA3 = 1;
        }
    } finally {
        rntA4 = rntA3 + 1;
    }
}

function fooRethrowFromFinally(): void {
    let er: Error|null = null;
    try {
        if (t1 != t2) throw new Er()
        ret += 1
    } catch (e) {
        ret += 10
        er = e;
    } finally {
        ret += 1000
        throw er!
    }
}

function main(): int {
    let suite = new arktest.ArkTestsuite("finally Clause")

    suite.addTest("No catch blocks, finally executes on normal completion", () => {
        let r: int = 1
        try {
            if (t1 == t2) throw new Er()
        } finally {
            r = 0
        }
        arktest.assertEQ(r, 0)
    })

    suite.addTest("Finally block with no error thrown", () => {
        let r: int = 1
        try {
            if (t1 == t2) throw new Er()
            r = 1
        } catch (e) {
        } finally {
            r = 0
        }
        arktest.assertEQ(r, 0)
    })

    suite.addTest("Empty finally block with no error thrown", () => {
        let r: int = 0
        try {
            if (t1 == t2) throw new Er()
        } catch (e) {
            r = 1
        } finally {
        }
        arktest.assertEQ(r, 0)
    })

    suite.addTest("Finally block with error thrown and caught", () => {
        let r: int = 0
        try {
            if (t1 != t2) throw new Er()
        } catch (e) {
            r = 1
        } finally {
            r = 0
        }
        arktest.assertEQ(r, 0)
    })

    suite.addTest("Error thrown, no suitable catch, propagates; finally runs", () => {
        ret = 0;
        try {
            fooNoSuitableCatch()
        } catch (e) {
            ret += 1000;
        }
        arktest.assertEQ(ret, 1100)
    })

    suite.addTest("Error thrown, with suitable catch, no propagation outside", () => {
        ret = 0;
        try {
            fooWithSuitableCatch()
        } catch (e) {
            ret += 10000;
        }
        arktest.assertEQ(ret, 1010)
    })

    suite.addTest("Error thrown, with suitable catch, re-thrown from catch", () => {
        ret = 0;
        try {
            fooRethrowFromCatch()
        } catch (e) {
            ret += 10000;
        }
        arktest.assertEQ(ret, 11010)
    })

    suite.addTest("Error thrown, with suitable catch, re-thrown from finally", () => {
        ret = 0;
        try {
            fooRethrowFromFinally()
        } catch (e) {
            ret += 10000;
        }
        arktest.assertEQ(ret, 11010)
    })

    suite.addTest("Throw from finally overrides throw from catch", () => {
        let caughtType = ""
        try {
            try {
                throw new Error()
            } catch(e) {
                throw new Er()
            } finally {
                throw new Er2()
            }
        } catch(e) {
            caughtType = (e instanceof Er2) ? "Er2" : "other"
        }
        arktest.assertEQ(caughtType, "Er2")
    })

    suite.addTest("Nested finally blocks both execute on error propagation", () => {
        let result = ""
        try {
            try {
                throw new Er()
            } finally {
                result += "inner"
            }
        } catch (e) {
            result += "-catch"
        } finally {
            result += "-outer"
        }
        arktest.assertEQ(result, "inner-catch-outer")
    })

    suite.addTest("Finally executes on return from try block, remaining try body is skipped", () => {
        fooReturnFromTry();
        arktest.assertEQ(rftA1, 1)   // set before return
        arktest.assertEQ(rftA2, 0)   // skipped due to return
        arktest.assertEQ(rftA3, 1)   // set in finally
    })

    suite.addTest("Return from nested try executes all enclosing finally blocks", () => {
        fooReturnFromNestedTry();
        arktest.assertEQ(rntA1, 1)   // set before return
        arktest.assertEQ(rntA2, 0)   // skipped due to return
        arktest.assertEQ(rntA3, 1)   // set in inner finally
        arktest.assertEQ(rntA4, 2)   // set in outer finally: rntA3 + 1
    })

    suite.addTest("Finally executes even when catch re-throws", () => {
        let result = ""
        try {
            try {
                throw new Er()
            } catch (e) {
                result += "catch"
                throw e
            } finally {
                result += "-finally"
            }
        } catch (e) {
            result += "-outer"
        }
        arktest.assertEQ(result, "catch-finally-outer")
    })

    suite.addTest("Finally executes on break from try block inside loop", () => {
        let result = ""
        for (let i = 0; i < 3; i++) {
            try {
                if (i == 1) break
                result += i.toString()
            } finally {
                result += "f"
            }
        }
        arktest.assertEQ(result, "0ff")
    })

    suite.addTest("Finally executes on continue from try block inside loop", () => {
        let result = ""
        for (let i = 0; i < 3; i++) {
            try {
                if (i == 1) continue
                result += i.toString()
            } finally {
                result += "f"
            }
        }
        arktest.assertEQ(result, "0ff2f")
    })

    suite.addTest("Exception thrown in finally when try completes normally propagates", () => {
        let result = ""
        try {
            try {
                result += "try"
            } finally {
                result += "-finally"
                throw new Er()
            }
        } catch (e) {
            result += "-caught"
        }
        arktest.assertEQ(result, "try-finally-caught")
    })

    return suite.run()
}
