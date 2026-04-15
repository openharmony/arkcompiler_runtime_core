/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

export namespace stvalue_workload {
  export let testField50: string = '';
  export let testField100: string = '';
  export let testField500: string = '';
  export let testField1000: string = '';
  export let testField5000: string = '';
  export let testField10000: string = '';
  export let testField50000: string = '';
  export let testNumberField: number = 0.0;
  export let testBooleanField: boolean = false;

  export function functionNoArgs(): number {
    return 42;
  }

  export function functionOneArgNumber(num: number): number {
    return num * 2;
  }

  export function functionOneArgString(str: string): number {
    return str.length;
  }

  export function functionFiveArgsNumber(n1: number, n2: number, n3: number, n4: number, n5: number): number {
    return n1 + n2 + n3 + n4 + n5;
  }

  export function functionFiveArgsString(s1: string, s2: string, s3: string, s4: string, s5: string): number {
    return s1.length + s2.length + s3.length + s4.length + s5.length;
  }
}

export class TestStaticClass {
  public static staticMethodNoArgs(): number {
    return 42;
  }

  public static staticMethodOneArgNumber(num: number): number {
    return num * 2;
  }

  public static staticMethodOneArgString(str: string): number {
    return str.length;
  }

  public static staticMethodFiveArgsNumber(n1: number, n2: number, n3: number, n4: number, n5: number): number {
    return n1 + n2 + n3 + n4 + n5;
  }

  public static staticMethodFiveArgsString(s1: string, s2: string, s3: string, s4: string, s5: string): number {
    return s1.length + s2.length + s3.length + s4.length + s5.length;
  }
}

export class TestInstanceClass {
  public methodNoArgs(): number {
    return 42;
  }

  public methodOneArgNumber(num: number): number {
    return num * 2;
  }

  public methodOneArgString(str: string): number {
    return str.length;
  }

  public methodFiveArgsNumber(n1: number, n2: number, n3: number, n4: number, n5: number): number {
    return n1 + n2 + n3 + n4 + n5;
  }

  public methodFiveArgsString(s1: string, s2: string, s3: string, s4: string, s5: string): number {
    return s1.length + s2.length + s3.length + s4.length + s5.length;
  }
}

export class TestFieldClass {
  public string50: string = '';
  public string100: string = '';
  public string500: string = '';
  public string1000: string = '';
  public string5000: string = '';
  public string10000: string = '';
  public string50000: string = '';
  public numberField: number = 0.0;
  public booleanField: boolean = false;
}

export class TestClass {
  public field: number = 42;
}
