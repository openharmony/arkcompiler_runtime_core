/*
 Copyright (c) 2023 Huawei Device Co., Ltd.
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 *
 http://www.apache.org/licenses/LICENSE-2.0
 *
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */


try {
  try {
    a = 1;
  } catch (e) {
    a;
  }
  if (a == "") {
    throw "null";
  }
  if (x > 100) {
    throw "max";
  } else {
    throw "min";
  }
}
catch (err) {
  func0(a, 10);
}
finally {
  console.log("error");
}

var car = ["B", "V", "p", "F", "A"];
var text = "";
var i;
for (i = 0; i < 5; i++) {
  text += car[i] + x + y;
}
function func0(x, y) {
  var a = x + y;
  return a;
}
function func5(a) {
  var a = 1;
  if (a) {
    return a;
  }
  else {
    a += 1;
  }
}

function func6(o) {
  var x = 42;

  try {
    throw o;
  }
  catch (e) {
    return x;
  }
}
