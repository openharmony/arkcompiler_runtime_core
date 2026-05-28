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
    Warning W0326: Attempting to cast type 'D' to 'B<String|Double>' can throw 'ClassCastError' exception at runtime
---*/

class B<T extends string|number|undefined> {
    fld: T
    constructor(p: T) { this.fld = p }
    process(p: T): T { return this.fld ?? p }
}

class D extends B<string|undefined> {
    constructor() { super(undefined) }
    override process(p: string|undefined): string {
        return 'x'
    }
}

function main() {
    let b: B<string|number> = new D() as B<string|number>;
    let n: number
    arktest.expectThrow(
        () => { n = b.process('a') as number },
        (e: Error) => e instanceof ClassCastError
    )
}
