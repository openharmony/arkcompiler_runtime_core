/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
  files: [class_module.ets]
---*/

import * as A from './class_module.ets'

class ThisFile {
    public static b: int = 56;
    public b: int = 32;
}

class ThisDiff {
    public static c: string = 's';
    public c: int = 6;
}

function main() : void {
    assertEQ(A.BothInt.a, 24);
    let bothInst = new A.BothInt;
    assertEQ(bothInst.a, 42);
    assertEQ(A.IntAndString.a, 23);
    let inst = new A.IntAndString;
    arktest.assertEQ(inst.a, '42');
    arktest.assertEQ(ThisFile.b, 56);
    let fileInst = new ThisFile;
    arktest.assertEQ(fileInst.b, 32);
    arktest.assertEQ(ThisDiff.c, 's');
    let diffInst = new ThisDiff;
    assertEQ(diffInst.c, 6);
}
