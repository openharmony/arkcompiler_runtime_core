/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

/*---
desc: 8.9 for-of Statements. !7 - iterable
tags: []
---*/

class StringIterator {
    private data: string;
    private index: number;

    constructor(data: string) {
        this.data = data;
        this.index = 0;
    }

    [Symbol.iterator]() {
        return {
            next: () => {
                if (this.index >= this.data.length) {
                    return {
                        value: this.data.charAt(this.index),
                        done: true,
                    }
                }
                if (this.index % 2 == 0) {
                    return {
                        value: this.data.charAt(this.index++),
                        done: false,
                    }
                } else {
                    this.index++;
                    return {
                        value: ' ',
                        done: false
                    }
                }

            },
        }
    }
}

class FooIterator<symbol> implements Iterator<symbol> {
  private index: number;
  private done: boolean;
  private values: String;

  constructor(values: string) {
      this.index = 0;
      this.done = false;
      this.values = values;
  }

  next(): IteratorResult<symbol, number | undefined> {
      if (this.done)
          return {
              done: this.done,
              value: undefined
          };

      if (this.index === this.values.length) {
          this.done = true;
          return {
              done: this.done,
              value: this.index
          };
      }

      const value = this.values.charAt[this.index];
      this.index++;
      return {
          done: false,
          value
      };
  }
}


function main() {
    let si = new StringIterator("hello_world")
    let expected: string = "h l o w r d"
    let result: string = "";
    for (const n of si) {
        result = result + n;
    }

    if (expected.length != result.length) {
        console.log("Hmm ... different length")
        console.log("Expected length : " + expected.length + " result length : " + result.length)
        return 1;
    }

    if (result !== expected) {
        console.log("Wrong context");
        console.log("result : " + result + " expected : " + expected);
        return 1;
    }

    console.log("Passed");
    return 0;
}

main();
