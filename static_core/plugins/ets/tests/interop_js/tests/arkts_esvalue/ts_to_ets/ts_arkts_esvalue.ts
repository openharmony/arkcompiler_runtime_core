/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

class MyObject {
    value: number;

    constructor(value: number) {
        this.value = value;
    }

    getDouble(): number {
        return this.value * 2;
    }
}

(globalThis as any).wrapobjTS = new MyObject(55);

(globalThis as any).unwrapobjTS = new MyObject(50);
function main(): void {
    let etsVm = globalThis.gtest.etsVm;

    const wrapobj = new MyObject(42);
    nativeWrapRef(wrapobj);
    nativeSaveRef(wrapobj)
    ASSERT_TRUE(etsVm.getFunction("Lts_arkts_esvalue/ETSGLOBAL;", "etsCheckWrappedPtr")());

    nativeWrapRef(wrapobjTS);
    ASSERT_TRUE(etsVm.getFunction("Lts_arkts_esvalue/ETSGLOBAL;", "etsCheckWrappedPtrFromTS")());

    ASSERT_TRUE(etsVm.getFunction("Lts_arkts_esvalue/ETSGLOBAL;", "etsCheckUnWrappedPtrFromTS")());
    
    const tag_lower = 0xABCD;
    const tag_upper = 0x1234;
    const safeObj = new MyObject(77);
    nativeWrapRefSafe(safeObj, tag_lower, tag_upper);
    nativeSaveRef(safeObj);
    ASSERT_TRUE(etsVm.getFunction('Lts_arkts_esvalue/ETSGLOBAL;', 'etsCheckWrappedPtrSafe')(tag_lower, tag_upper));

    // 2. Failure Case: Mismatching Tags
    ASSERT_TRUE(!etsVm.getFunction('Lts_arkts_esvalue/ETSGLOBAL;', 'etsCheckWrappedPtrSafe')( 0, 0));

    // 3. Failure Case: Wrapped without Tag
    const normalObj = new MyObject(88);
    nativeWrapRef(normalObj);
    nativeSaveRef(normalObj);
    ASSERT_TRUE(!etsVm.getFunction('Lts_arkts_esvalue/ETSGLOBAL;', 'etsCheckWrappedPtrSafe')(tag_lower, tag_upper));

    // 4. Conner case
    const zeroObj = new MyObject(0);
    const zero_tag = 0x0000;
    nativeWrapRefSafe(zeroObj, zero_tag, zero_tag);
    nativeSaveRef(zeroObj);
    ASSERT_TRUE(etsVm.getFunction('Lts_arkts_esvalue/ETSGLOBAL;', 'etsCheckWrappedPtrSafe')(zero_tag, zero_tag));
    
    const maxObj = new MyObject(127);
    const max_tag = 0xFFFF;
    nativeWrapRefSafe(maxObj, max_tag, max_tag);
    nativeSaveRef(maxObj);
    ASSERT_TRUE(etsVm.getFunction('Lts_arkts_esvalue/ETSGLOBAL;', 'etsCheckWrappedPtrSafe')(max_tag, max_tag));

    // 5. Mismatch
    const errorObj = new MyObject(99);
    nativeWrapRefSafe(errorObj, tag_lower, tag_upper);
    nativeSaveRef(errorObj);
    ASSERT_TRUE(!etsVm.getFunction('Lts_arkts_esvalue/ETSGLOBAL;', 'etsCheckWrappedPtrSafe')(tag_lower, tag_lower));
    ASSERT_TRUE(!etsVm.getFunction('Lts_arkts_esvalue/ETSGLOBAL;', 'etsCheckWrappedPtrSafe')(tag_upper, tag_upper));
}

(globalThis as any).nativeSaveRef("temp");

const obj = {};

(globalThis as any).nativeWrapRef(obj);
main();
