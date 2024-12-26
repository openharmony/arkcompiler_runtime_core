/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

// This test check by WeakRef on JS side that STS object that is transferred 
// from STS -> JS and then from JS -> STS creating cyclic reference -
// can be collected by GC

import { interop } from './gc_test_common_ts';

function main(): void {
    let arr: Object[] = [];
    for (let i = 0; i < 10; i++) {
        arr.push(interop.GetSTSObject());
    }
    let wr = new WeakRef(arr);
    interop.AddPandaArray(arr);
    arr = null;
    interop.RunInteropGC();
    interop.RunPandaGC();
    let gcId = globalThis.ArkTools.GC.startGC("full");
    globalThis.ArkTools.GC.waitForFinishGC(gcId);
    if (wr.deref()) {
        throw Error('Object is not collected by GC!');
    }
}
main();
