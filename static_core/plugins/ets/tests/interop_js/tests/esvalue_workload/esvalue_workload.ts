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

export class Foo {
  public string50: string;
  public string100: string;
  public string500: string;
  public string1000: string;
  public string5000: string;
  public string10000: string;
  public string50000: string;
  public booleanField: boolean;
  public numberField: number;
  public objectField: object;

  constructor() {
    let stringArray50 = new Array<string>(50);
    this.string50 = stringArray50.fill('a').join('');

    let stringArray100 = new Array<string>(100);
    this.string100 = stringArray100.fill('a').join('');

    let stringArray500 = new Array<string>(500);
    this.string500 = stringArray500.fill('a').join('');

    let stringArray1000 = new Array<string>(1000);
    this.string1000 = stringArray1000.fill('a').join('');

    let stringArray5000 = new Array<string>(5000);
    this.string5000 = stringArray5000.fill('a').join('');

    let stringArray10000 = new Array<string>(10000);
    this.string10000 = stringArray10000.fill('a').join('');

    let stringArray50000 = new Array<string>(50000);
    this.string50000 = stringArray50000.fill('a').join('');

    this.booleanField = true;
    this.numberField = 42.5;
    this.objectField = new Object();
  }

  public methodNoArgs(): string {
    return "noArgs";
  }

  public methodOneArgNumber(num: number): string {
    return "number:" + num;
  }

  public methodOneArgString(str: string): string {
    return "string:" + str;
  }

  public methodOneArgObject(obj: object): string {
    return "object:" + JSON.stringify(obj);
  }

  public methodFiveArgsNumber(n1: number, n2: number, n3: number, n4: number, n5: number): string {
    return "fiveNumbers:" + n1 + "," + n2 + "," + n3 + "," + n4 + "," + n5;
  }

  public methodFiveArgsString(s1: string, s2: string, s3: string, s4: string, s5: string): string {
    return "fiveStrings:" + s1 + "," + s2 + "," + s3 + "," + s4 + "," + s5;
  }

  public methodFiveArgsObject(o1: object, o2: object, o3: object, o4: object, o5: object): string {
    return "fiveObjects:" + JSON.stringify([o1, o2, o3, o4, o5]);
  }

}

export let fooInstance = new Foo();

export function functionNoArgs(): string {
  return "functionNoArgs";
}

export function functionOneArgNumber(num: number): string {
  return "functionNumber:" + num;
}

export function functionOneArgString(str: string): string {
  return "functionString:" + str;
}

export function functionOneArgObject(obj: object): string {
  return "functionObject:" + JSON.stringify(obj);
}

export function functionFiveArgsNumber(n1: number, n2: number, n3: number, n4: number, n5: number): string {
  return "functionFiveNumbers:" + n1 + "," + n2 + "," + n3 + "," + n4 + "," + n5;
}

export function functionFiveArgsString(s1: string, s2: string, s3: string, s4: string, s5: string): string {
  return "functionFiveStrings:" + s1 + "," + s2 + "," + s3 + "," + s4 + "," + s5;
}

export function functionFiveArgsObject(o1: object, o2: object, o3: object, o4: object, o5: object): string {
  return "functionFiveObjects:" + JSON.stringify([o1, o2, o3, o4, o5]);
}