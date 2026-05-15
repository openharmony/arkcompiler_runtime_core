/*
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

 /*---
desc: >-
    never routines override all possible return types, including bytecode primitives, void and never itself.
---*/

{% for c in cases %}

class Base{{loop.index}} {
    methodOne(): {{c[0]}} {
        return {{c[2]}}
    }
}


interface BaseI{{loop.index}} {
    methodTwo(): {{c[1]}} {
        return {{c[2]}}
    }
}

class Derived{{loop.index}} extends Base{{loop.index}} implements BaseI{{loop.index}} {
    override methodOne(): never {
        throw new Error('Never')
    }
    methodTwo(): never {
        throw new Error('Never')
    }
}
{%- endfor %}


function main(): int {

    let testSuite = new arktest.ArkTestsuite("Never_overriding");

    {% for c in cases %}
        testSuite.addTest("override_{{loop.index}}", () => {
            let expectedException = (e: Error): boolean => {
                return (e.message == 'Never')
            }
            //as base class
            arktest.expectThrow(
                () => { (new Derived{{loop.index}}() as Base{{loop.index}}).methodOne() },
                expectedException
            )
            //as interface
            arktest.expectThrow(
                () => { (new Derived{{loop.index}}() as BaseI{{loop.index}}).methodTwo() },
                expectedException
            )
        });
    {%- endfor %}

    return testSuite.run();
}
