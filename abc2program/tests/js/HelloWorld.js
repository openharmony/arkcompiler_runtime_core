/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import {a} from './a'
import * as b from './b'
export let c;
export * from './d'
export {e} from './e'

class HelloWorld {
  str = 'HelloWorld';
}

class Lit {
  *lit() {}
}

msg : string = "";

function foo() {
  try {
    varA = 11;
    x = 22;
    try {
      varA = 1;
    } catch (e) {
      msg = "inner catch";
      print(msg);
    }
    if (varA == "") {
      throw "null";
    }
    if (x > 100) {
      throw "max"
    } else {
      throw "min";
    }
  }
  catch (err) {
    masg = "outter catch";
    print(msg);
  }
  finally {
    msg = "error";
    print(msg);
  }
}

function goo() {

}

function hoo () {
  varA = 1.23;
}

foo();

print(goo.toString());

hoo();