/*
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

/*---
desc: Using 'this' for method chaining in generic class.
assert: The keyword this enables fluent API pattern in generic classes.
---*/

class Builder<T> {
  private value: T;

  public setValue(val: T): Builder<T> {
    this.value = val;
    return this;
  }

  public process(): Builder<T> {
    return this;
  }

  public build(): T {
    return this.value;
  }
}

function main(): void {
  let result: string = new Builder<string>()
    .setValue("hello")
    .process()
    .build();

  arktest.assertEQ(result, "hello");

  let numResult: int = new Builder<int>()
    .setValue(11)
    .process()
    .build();

  arktest.assertEQ(numResult, 11);

  let builder: Builder<string> = new Builder<string>()
    .setValue("test")
    .process();

  arktest.assertTrue(builder instanceof Builder);
  arktest.assertEQ(builder.build(), "test");
}
