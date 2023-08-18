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


function func(num) {
  if (num > 0) {
    num = 0;
  } else {
    num += 1;
  }
  return num;
}

function func1(num1, num2) {
  const num = 1;
  try {
    if (num1 > num2) {
      num1 += num;
      console.log('succes.');
    }
    else {
      throw new Error('the number is low.');
    }
  }
  catch (err) {
    console.log('error message: ' + err);
  }
}

function func2(x, y) {
  var car = ["B", "V", "p", "F", "A"];
  var text = "";
  var i;
  for (i = 0; i < 5; i++) {
    text += car[i] + x + y;
  }
  return text;
}

function func3(a, b, c, d, e) {
  return a + b + c + d + e;
}

function func4(x, y) {
  var a = x + y;
  var b = x - y;
  var c = x * y;
  var d = x / y;
  var e = x % y;
  return func3(e, d, c, b, a);
}

function func5() {
  try {
    try {
      a = 1;
    } catch (e) {
      a;
    }
    if (a > 0) {
      a += 1;
    } else {
      throw new Error('the number is low.');
    }
  } catch (e) {
    print(e);
  }
}

