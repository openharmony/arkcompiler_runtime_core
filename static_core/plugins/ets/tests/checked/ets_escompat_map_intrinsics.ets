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

class MapEntry<K, V> {
    key: K
    val: V

    constructor(key: K, val: V) {
        this.key = key
        this.val = val
    }
}

class MyMap<K, V> extends Map<K, V> {
    public arr: Array<MapEntry<K, V>> | undefined

    set(key: K, val: V): this {
        if (this.arr == undefined)
            this.arr = new Array<MapEntry<K, V>>()
        const newEntry = new MapEntry<K, V>(key, val)
        this.arr?.push(newEntry)
        return this;
    }

    public has(key: K): boolean {
        let elem = this.arr?.pop()
        if (elem?.key == key)
            return false
        return true;
    }

    public get(key: K): V | undefined {
        let elem = this.arr?.pop()
        if (elem?.key == key)
            return elem?.val
        return undefined
    }

    public delete(key: K): boolean {
        let elem = this.arr?.pop()
        if (elem?.key == key)
            return false
        return true
    }
}


//! CHECKER       AOT: Map<boolean, double> overrided intrinsics
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*(test_map|MapHas|MapGet|MapDelete)_.*'", entry: "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_bool_double_overrided1"
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::__noinline__MapHas_bool_double"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.MapHas"
//! INST          /CallVirtual.*escompat\.Map::has/
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::__noinline__MapGet_bool_double"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.MapGet"
//! INST          /CallVirtual.*escompat\.Map::get/
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::__noinline__MapDelete_bool_double"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.MapDelete"
//! INST          /CallVirtual.*escompat\.Map::delete/

//! CHECKER       AOT PGO: Map<boolean, double> overrided intrinsics
//! SKIP_IF       @architecture == "arm32"
//! RUN_PGO_PROF entry: "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_bool_double_overrided1"
//! RUN_PGO_PAOC options: "--compiler-regex='.*(test_map|MapHas|MapGet|MapDelete)_.*'"
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::__noinline__MapHas_bool_double"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.MapHas"
//! INST          /CallVirtual.*escompat\.Map::has/
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::__noinline__MapGet_bool_double"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.MapGet"
//! INST          /CallVirtual.*escompat\.Map::get/
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::__noinline__MapDelete_bool_double"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.MapDelete"
//! INST          /CallVirtual.*escompat\.Map::delete/

//! CHECKER       JIT: Map<boolean, double> overrided intrinsics
//! RUN           force_jit: true, options: "--compiler-regex='.*(test_map|MapHas|MapGet|MapDelete)_.*'", entry: "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_bool_double_overrided1"
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::__noinline__MapHas_bool_double"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::__noinline__MapGet_bool_double"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapGet"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapGet"
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::__noinline__MapDelete_bool_double"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapDelete"

function test_map_bool_double_overrided1() {
    let map: Map<boolean, double> = new Map<boolean, double>()
    map.set(false, 1.5)
    arktest.assertFalse(__noinline__MapHas_bool_double(map))
    arktest.assertEQ(__noinline__MapGet_bool_double(map), 1.5)
    arktest.assertFalse(__noinline__MapDelete_bool_double(map))
}


//! CHECKER       AOT: MyMap<boolean, double> overrided intrinsics
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*(test_map|MapHas|MapGet|MapDelete)_.*'", entry: "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_bool_double_overrided2"
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::__noinline__MapHas_bool_double"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.MapHas"
//! INST          /CallVirtual.*escompat\.Map::has/
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::__noinline__MapGet_bool_double"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.MapGet"
//! INST          /CallVirtual.*escompat\.Map::get/
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::__noinline__MapDelete_bool_double"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.MapDelete"
//! INST          /CallVirtual.*escompat\.Map::delete/

//! CHECKER       AOT PGO: MyMap<boolean, double> overrided intrinsics
//! SKIP_IF       @architecture == "arm32"
//! RUN_PGO_PROF entry: "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_bool_double_overrided2"
//! RUN_PGO_PAOC options: "--compiler-regex='.*(test_map|MapHas|MapGet|MapDelete)_.*'"
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::__noinline__MapHas_bool_double"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.MapHas"
//! INST          /CallVirtual.*ets_escompat_map_intrinsics\.MyMap::has/
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::__noinline__MapGet_bool_double"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.MapGet"
//! INST          /CallVirtual.*ets_escompat_map_intrinsics\.MyMap::get/
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::__noinline__MapDelete_bool_double"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.MapDelete"
//! INST          /CallVirtual.*ets_escompat_map_intrinsics\.MyMap::delete/

//! CHECKER       JIT: MyMap<boolean, double> overrided intrinsics
//! RUN           force_jit: true, options: "--compiler-regex='.*(test_map|MapHas|MapGet|MapDelete)_.*'", entry: "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_bool_double_overrided2"
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::__noinline__MapHas_bool_double"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.MapHas"
//! INST          /CallVirtual.*escompat\.Map::has/
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::__noinline__MapGet_bool_double"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.MapGet"
//! INST          /CallVirtual.*escompat\.Map::get/
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::__noinline__MapDelete_bool_double"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.MapDelete"
//! INST          /CallVirtual.*escompat\.Map::delete/

function test_map_bool_double_overrided2() {
    let map: MyMap<boolean, double> = new MyMap<boolean, double>()
    map.set(false, 1.5)
    arktest.assertTrue(__noinline__MapHas_bool_double(map))
    arktest.assertEQ(__noinline__MapGet_bool_double(map), undefined)
    arktest.assertTrue(__noinline__MapDelete_bool_double(map))
}

function __noinline__MapHas_bool_double(map: Map<boolean, double>): boolean {
    return map.has(true);
}
function __noinline__MapGet_bool_double(map: Map<boolean, double>): double | undefined {
    return map.get(false);
}
function __noinline__MapDelete_bool_double(map: Map<boolean, double>): boolean {
    return map.delete(true);
}


//! CHECKER       AOT: Map intrinsics
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*test_map_.*'", entry: "ets_escompat_map_intrinsics.ETSGLOBAL::main"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_bool_double"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_char_double"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_byte_int"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_short_double"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_int_double"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_long_double"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_float_double"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_double_double"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_double_number"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_string_number"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"

//! CHECKER       AOT PGO: Map intrinsics
//! SKIP_IF       @architecture == "arm32"
//! RUN_PGO_PROF entry: "ets_escompat_map_intrinsics.ETSGLOBAL::main"
//! RUN_PGO_PAOC options: "--compiler-regex='.*test_map_.*'"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_bool_double"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_char_double"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_byte_int"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_short_double"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_int_double"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_long_double"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_float_double"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_double_double"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_double_number"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_string_number"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"

//! CHECKER       JIT: Map intrinsics
//! RUN           force_jit: true, options: "--compiler-regex='.*test_map_.*'", entry: "ets_escompat_map_intrinsics.ETSGLOBAL::main"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_bool_double"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_char_double"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_byte_int"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_short_double"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_int_double"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_long_double"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_float_double"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_double_double"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_double_number"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"
//!
//! METHOD        "ets_escompat_map_intrinsics.ETSGLOBAL::test_map_string_number"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.MapHas"
//! INST_NOT      "Intrinsic.MapGet"
//! INST_NOT      "Intrinsic.MapDelete"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.MapHas"
//! INST          "Intrinsic.MapGet"
//! INST          "Intrinsic.MapDelete"

function test_map_bool_double() {
    let map: Map<boolean, double> = new Map<boolean, double>()
    map.set(false, 1.5)
    arktest.assertTrue(map.has(false))
    arktest.assertEQ(map.get(false), 1.5)
    arktest.assertFalse(map.has(true))
    arktest.assertEQ(map.get(true), undefined)

    map.set(true, 2.5)
    arktest.assertTrue(map.delete(true))
    arktest.assertFalse(map.delete(true))
}

function test_map_char_double() {
    let map: Map<char, double> = new Map<char, double>()
    map.set(c'A', 1.5)
    arktest.assertTrue(map.has(c'A'))
    arktest.assertEQ(map.get(c'A'), 1.5)
    arktest.assertFalse(map.has(c'B'))
    arktest.assertEQ(map.get(c'B'), undefined)

    map.set(c'B', 2.5)
    arktest.assertTrue(map.delete(c'B'))
    arktest.assertFalse(map.delete(c'B'))
}

function test_map_byte_int() {
    let map: Map<byte, int> = new Map<byte, int>()
    map.set((1).toByte(), 1)
    arktest.assertTrue(map.has((1).toByte()))
    arktest.assertEQ(map.get((1).toByte()), 1)
    arktest.assertFalse(map.has((2).toByte()))
    arktest.assertEQ(map.get((2).toByte()), undefined)

    map.set((2).toByte(), 2)
    arktest.assertTrue(map.delete((2).toByte()))
    arktest.assertFalse(map.delete((2).toByte()))
}

function test_map_short_double() {
    let map: Map<short, double> = new Map<short, double>()
    map.set((1).toShort(), 1.5)
    arktest.assertTrue(map.has((1).toShort()))
    arktest.assertEQ(map.get((1).toShort()), 1.5)
    arktest.assertFalse(map.has((2).toShort()))
    arktest.assertEQ(map.get((2).toShort()), undefined)

    map.set((2).toShort(), 2.5)
    arktest.assertTrue(map.delete((2).toShort()))
    arktest.assertFalse(map.delete((2).toShort()))
}

function test_map_int_double(): void {
    let map: Map<int, double> = new Map<int, double>()
    map.set(1, 1.5)
    arktest.assertTrue(map.has(1))
    arktest.assertEQ(map.get(1), 1.5)
    arktest.assertFalse(map.has(2))
    arktest.assertEQ(map.get(2), undefined)

    map.set(2, 2.5)
    arktest.assertTrue(map.delete(2))
    arktest.assertFalse(map.delete(2))
}

function test_map_long_double() {
    let map: Map<long, double> = new Map<long, double>()
    map.set(1, 1.5)
    arktest.assertTrue(map.has(1))
    arktest.assertEQ(map.get(1), 1.5)
    arktest.assertFalse(map.has(2))
    arktest.assertEQ(map.get(2), undefined)

    map.set(2, 2.5)
    arktest.assertTrue(map.delete(2))
    arktest.assertFalse(map.delete(2))
}

function test_map_float_double() {
    let map: Map<float, double> = new Map<float, double>()
    map.set(7.2f, 7.5)
    arktest.assertTrue(map.has(7.2f))
    arktest.assertEQ(map.get(7.2f), 7.5)
    arktest.assertFalse(map.has(2))
    arktest.assertEQ(map.get(2.5f), undefined)

    map.set(2.5f, 2.5)
    arktest.assertTrue(map.delete(2.5f))
    arktest.assertFalse(map.delete(2.5f))
}

function test_map_double_double() {
    let map: Map<double, double> = new Map<double, double>()
    map.set(1.5, 1.5)
    arktest.assertTrue(map.has(1.5))
    arktest.assertEQ(map.get(1.5), 1.5)
    arktest.assertFalse(map.has(2.1))
    arktest.assertEQ(map.get(2.1), undefined)

    map.set(2.1, 2.5)
    arktest.assertTrue(map.delete(2.1))
    arktest.assertFalse(map.delete(2.1))
}

function test_map_double_number() {
    let map: Map<double, number> = new Map<double, number>()
    map.set(7.2, 7.5)
    arktest.assertTrue(map.has(7.2))
    arktest.assertEQ(map.get(7.2), 7.5)
    arktest.assertFalse(map.has(2.5))
    arktest.assertEQ(map.get(2.5), undefined)

    map.set(2.5, 2.5)
    arktest.assertTrue(map.delete(2.5))
    arktest.assertFalse(map.delete(2.5))
}

function test_map_string_number() {
    let map: Map<string, number> = new Map<string, number>()
    arktest.assertFalse(map.has(""))
    map.set("", 0.5)
    map.set("abc", 0.5)
    map.set("defgh", 1.5)
    arktest.assertTrue(map.has(""))
    arktest.assertEQ(map.get(""), 0.5)
    arktest.assertTrue(map.has("abc"))
    arktest.assertEQ(map.get("abc"), 0.5)
    arktest.assertTrue(map.has("defgh"))
    arktest.assertEQ(map.get("defgh"), 1.5)
    arktest.assertFalse(map.has("erty"))
    arktest.assertEQ(map.get("erty"), undefined)

    map.set("erty", 2.5)
    arktest.assertTrue(map.delete("erty"))
    arktest.assertFalse(map.delete("erty"))
}

function main(): void {
    test_map_bool_double()
    test_map_char_double()
    test_map_byte_int()
    test_map_short_double()
    test_map_int_double()
    test_map_long_double()
    test_map_float_double()
    test_map_double_double()
    test_map_double_number()
    test_map_string_number()
}
