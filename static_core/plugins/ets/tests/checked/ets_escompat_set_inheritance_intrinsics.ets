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

class MySet<K> extends Set<K> {
    public arr: Array<K> | undefined

    add(key: K): this {
        if (this.arr == undefined)
            this.arr = new Array<K>()
        this.arr?.push(key)
        return this;
    }

    public has(key: K): boolean {
        let elem = this.arr?.pop()
        if (elem == key)
            return false
        return true;
    }

    public delete(key: K): boolean {
        let elem = this.arr?.pop()
        if (elem == key)
            return false
        return true
    }
}

//! CHECKER       AOT: MySet<boolean> overrided intrinsics
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*(test_set|SetHas|SetDelete)_.*'", entry: "ets_escompat_set_inheritance_intrinsics.ETSGLOBAL::test_set_bool_double_overrided2"
//! METHOD        "ets_escompat_set_inheritance_intrinsics.ETSGLOBAL::__noinline__SetHas_bool"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.SetHas"
//! INST          /CallVirtual.*escompat\.Set::has/
//! METHOD        "ets_escompat_set_inheritance_intrinsics.ETSGLOBAL::__noinline__SetDelete_bool"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.SetDelete"
//! INST          /CallVirtual.*escompat\.Set::delete/

//! CHECKER       AOT PGO: MySet<boolean> overrided intrinsics
//! SKIP_IF       @architecture == "arm32"
//! RUN_PGO_PROF entry: "ets_escompat_set_inheritance_intrinsics.ETSGLOBAL::test_set_bool_double_overrided2"
//! RUN_PGO_PAOC options: "--compiler-regex='.*(test_set|SetHas|SetDelete)_.*'"
//! METHOD        "ets_escompat_set_inheritance_intrinsics.ETSGLOBAL::__noinline__SetHas_bool"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.SetHas"
//! INST          /ets_escompat_set_inheritance_intrinsics\.MySet::has/
//! METHOD        "ets_escompat_set_inheritance_intrinsics.ETSGLOBAL::__noinline__SetDelete_bool"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.SetDelete"
//! INST          /ets_escompat_set_inheritance_intrinsics\.MySet::delete/

//! CHECKER       JIT: MySet<boolean> overrided intrinsics
//! RUN           force_jit: true, options: "--compiler-regex='.*(test_set|SetHas|SetDelete)_.*'", entry: "ets_escompat_set_inheritance_intrinsics.ETSGLOBAL::test_set_bool_double_overrided2"
//! METHOD        "ets_escompat_set_inheritance_intrinsics.ETSGLOBAL::__noinline__SetHas_bool"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.SetHas"
//! INST          /CallVirtual.*escompat\.Set::has/
//! METHOD        "ets_escompat_set_inheritance_intrinsics.ETSGLOBAL::__noinline__SetDelete_bool"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.SetDelete"
//! INST          /CallVirtual.*escompat\.Set::delete/

function test_set_bool_double_overrided2() {
    let set: MySet<boolean> = new MySet<boolean>()
    set.add(false)
    arktest.assertTrue(__noinline__SetHas_bool(set))
    arktest.assertTrue(__noinline__SetDelete_bool(set))
}

function __noinline__SetHas_bool(set: Set<boolean>): boolean {
    return set.has(true);
}
function __noinline__SetDelete_bool(set: Set<boolean>): boolean {
    return set.delete(true);
}
