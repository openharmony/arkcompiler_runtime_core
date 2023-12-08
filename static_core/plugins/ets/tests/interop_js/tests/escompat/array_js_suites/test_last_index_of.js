/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

const { etsVm, getTestModule } = require('escompat.test.js');

const etsMod = getTestModule('escompat_test');
const GCJSRuntimeCleanup = etsMod.getFunction('GCJSRuntimeCleanup');
const FooClass = etsMod.getClass('FooClass');
const CreateEtsSample = etsMod.getFunction('Array_CreateEtsSample');
const TestJSLastIndexOf = etsMod.getFunction('Array_TestJSLastIndexOf');

{ // Test JS Array<FooClass>
  TestJSLastIndexOf(new Array(new FooClass('zero'), new FooClass('one')));
}

{ // Test ETS Array<Object>
  let arr = CreateEtsSample();
  arr.push('foo');

  ASSERT_EQ(arr.lastIndexOf('foo'), 2);
  ASSERT_EQ(arr.lastIndexOf('not in array'), -1);

  // NOTE(oignatenko) uncomment below after interop will be supported for this method signature
  // ASSERT_EQ(arr.lastIndexOf('foo', 1), 2);
  // ASSERT_EQ(arr.lastIndexOf('not in array', 1), -1);
}

GCJSRuntimeCleanup();
