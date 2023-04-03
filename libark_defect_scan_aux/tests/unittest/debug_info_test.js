/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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


function func1(o1, o2) {
  console.log(o1 + o2);
}

async function func2() { }

let func3 = (o1, o2, o3, o4, o5) => { };

function foo(o1, o2) {
  const a = 1;
  const b = 2;
  let c = 0;
  if (o1.x > a + b) {
    c = o1.x;
  } else if (o2.x < a + b) {
    c = o2.x;
  } else {
    c = a + b + func2();
  }

  let arr = [a, b];
  let m = '123'; func1(...arr);  // written on the same line for testing

  func3(a, b, c, c, m);
}

class DataBase {
  constructor(x) {
    this.x = x;
  }

  add(o) {
    this.x += o;
  }

  sub(o) {
    this.x -= o;
  }
}

function bar(o) {
  class Data extends DataBase {
    constructor(x, y) {
      super(x);
      this.y = y;
    }
  }

  let data = new Data();
  // multiple blank lines here are for testing



  data.sub(o);

  let b = data.x;
  if (o) {
    b += 1;
    console.log(b);
  }
}