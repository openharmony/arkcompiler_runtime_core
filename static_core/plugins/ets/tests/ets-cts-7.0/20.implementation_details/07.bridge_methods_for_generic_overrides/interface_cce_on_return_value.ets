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
desc: ClassCastError on return value from bridge method
expected_out: |-
    Warning W369037: Cast expression `as B<Boolean|Double>` is type unsafe. 'ClassCastError' exception can be thrown at runtime
---*/

interface B<T extends boolean|number> {
    process(p: T): T { return p }
}

class D implements B<boolean> {
    fld: boolean
    override process(p: boolean): boolean {
        return true
    }
}

function main() {
    let b: B<boolean|number> = new D() as B<boolean|number>;
    let n: number
    arktest.expectThrow(
        () => { n = b.process(true) as number },
        (e: Error) => e instanceof ClassCastError
    )
}
