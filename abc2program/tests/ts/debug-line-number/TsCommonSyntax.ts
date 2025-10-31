/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

class A {
}

class B {
  x = 1;
}

class C extends A {
}

class D {
  get value() {
    return 42;
  }
  set value(v) {
  }
}

class E {
  static x = 1;
}

class F {
  add() {
    return 0;
  }
}

class G {
  @TimeLogger
  add(a: number, b: number): number {
    return a + b;
  }
}