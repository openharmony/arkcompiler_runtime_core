/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

import { makeEnum, jsEnum, isEq } from "js_code"

function Test1(): boolean {
    let a = makeEnum(['red', 'green', 'blue'])
    if(a == null) {
      return false
    } else {
      return true
    }
}

function Test2(): boolean {
    let a = makeEnum(['red', 'green', 'blue'])
    if(a.red == null || a.green == null || a.blue == null) {
      return false
    } else {
      return true
    }
}

function Test3(): boolean {
    let d = jsEnum()
    if(d == null) {
      return false
    } else {
      return true
    }
}


function Test4(): boolean {
    let d = jsEnum()
    if(d.Up as string == 'Up' && d.Down as string == 'Down') {
      return true
    } else {
      return false
    }
}

function Test5(): boolean {
    let d = jsEnum()
    let u = d.Up
    if(isEq(u, d.Up) as boolean) {
      return true
    } else {
      return false
    }
}

function Test6(): boolean {
    let d = jsEnum()
    let u = d.Up
    if(isEq('Up', u  as string) as boolean) {
      return true
    } else {
      return false
    }
}