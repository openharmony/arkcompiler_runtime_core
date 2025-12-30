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

//! CHECKER       AOT: don't inline external Set<boolean>
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*(test_set|SetHas|SetDelete)_.*'", entry: "ets_escompat_set_inlining_overrided.ETSGLOBAL::test_set_bool_double_overrided1"
//! EVENT_NOT     /Inline,ets_escompat_set_inlining_overrided\.ETSGLOBAL::__noinline__SetHas_bool,std\.core\.Set::has,.*,SUCCESS/
//! EVENT_NOT     /Inline,ets_escompat_set_inlining_overrided\.ETSGLOBAL::__noinline__SetDelete_bool,std\.core\.Set::delete,.*,SUCCESS/
//! METHOD        "ets_escompat_set_inlining_overrided.ETSGLOBAL::__noinline__SetHas_bool"
//! PASS_AFTER    "Inline"
//! INST          /CallVirtual.*std\.core\.Set::has/
//! METHOD        "ets_escompat_set_inlining_overrided.ETSGLOBAL::__noinline__SetDelete_bool"
//! PASS_AFTER    "Inline"
//! INST          /CallVirtual.*std\.core\.Set::delete/

//! CHECKER       AOT PGO: don't inline external Set<boolean>
//! SKIP_IF       @architecture == "arm32"
//! RUN_PGO_PROF entry: "ets_escompat_set_inlining_overrided.ETSGLOBAL::test_set_bool_double_overrided1"
//! RUN_PGO_PAOC options: "--compiler-regex='.*(test_set|SetHas|SetDelete)_.*' --compiler-inline-external-methods-aot=false"
//! EVENT_NOT     /Inline,ets_escompat_set_inlining_overrided\.ETSGLOBAL::__noinline__SetHas_bool,std\.core\.Set::has,.*,SUCCESS/
//! EVENT_NOT     /Inline,ets_escompat_set_inlining_overrided\.ETSGLOBAL::__noinline__SetDelete_bool,std\.core\.Set::delete,.*,SUCCESS/
//! METHOD        "ets_escompat_set_inlining_overrided.ETSGLOBAL::__noinline__SetHas_bool"
//! PASS_AFTER    "Inline"
//! INST          /CallVirtual.*std\.core\.Set::has/
//! METHOD        "ets_escompat_set_inlining_overrided.ETSGLOBAL::__noinline__SetDelete_bool"
//! PASS_AFTER    "Inline"
//! INST          /CallVirtual.*std\.core\.Set::delete/

//! CHECKER       JIT: inline Map methods inside Set<boolean>
//! RUN           force_jit: true, options: "--compiler-regex='.*(test_set|SetHas|SetDelete)_.*'", entry: "ets_escompat_set_inlining_overrided.ETSGLOBAL::test_set_bool_double_overrided1"
//! EVENT         /Inline,ets_escompat_set_inlining_overrided\.ETSGLOBAL::__noinline__SetHas_bool,std\.core\.Set::has,.*,SUCCESS/
//! EVENT         /Inline,ets_escompat_set_inlining_overrided\.ETSGLOBAL::__noinline__SetDelete_bool,std\.core\.Set::delete,.*,SUCCESS/
//! METHOD        "ets_escompat_set_inlining_overrided.ETSGLOBAL::__noinline__SetHas_bool"
//! PASS_AFTER    "Inline"
//! INST          /Call.*Inlined.*std\.core\.Set::has/
//! INST          /Call.*Inlined.*std\.core\.Map::has/
//! METHOD        "ets_escompat_set_inlining_overrided.ETSGLOBAL::__noinline__SetDelete_bool"
//! PASS_AFTER    "Inline"
//! INST          /Call.*Inlined.*std\.core\.Set::delete/
//! INST          /Call.*Inlined.*std\.core\.Map::delete/

function test_set_bool_double_overrided1() {
    let set: Set<boolean> = new Set<boolean>()
    set.add(false)
    arktest.assertFalse(__noinline__SetHas_bool(set))
    arktest.assertFalse(__noinline__SetDelete_bool(set))
}

function __noinline__SetHas_bool(set: Set<boolean>): boolean {
    return set.has(true);
}
function __noinline__SetDelete_bool(set: Set<boolean>): boolean {
    return set.delete(true);
}
