/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * You may not use this file except in compliance with the License.
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

let keepAlive: object[] = [];

function testCountInstancesOfClass() {
  class TestClassA1 {}
  let type = Type.from<TestClassA1>() as ClassType;
  let init = RuntimeDebug.countInstancesOfClass(type);
  keepAlive.push(new TestClassA1());
  keepAlive.push(new TestClassA1());
  let count = RuntimeDebug.countInstancesOfClass(type);
  arktest.assertLE(2, count);
  arktest.assertLE(count, init + 2);
}

function testInheritanceBaseClassCount() {
  class TestClassA2 {}
  class TestSubClassB2 extends TestClassA2 {}
  let type = Type.from<TestClassA2>() as ClassType;
  let init = RuntimeDebug.countInstancesOfClass(type);
  keepAlive.push(new TestClassA2());
  keepAlive.push(new TestClassA2());
  keepAlive.push(new TestSubClassB2());
  let count = RuntimeDebug.countInstancesOfClass(type);
  arktest.assertLE(2, count);
  arktest.assertLE(count, init + 2);
}

function testInheritanceSubClassCount() {
  class TestClassA3 {}
  class TestSubClassB3 extends TestClassA3 {}
  let type = Type.from<TestSubClassB3>() as ClassType;
  let init = RuntimeDebug.countInstancesOfClass(type);
  keepAlive.push(new TestSubClassB3());
  keepAlive.push(new TestSubClassB3());
  keepAlive.push(new TestClassA3());
  let count = RuntimeDebug.countInstancesOfClass(type);
  arktest.assertLE(2, count);
  arktest.assertLE(count, init + 2);
}

function testZeroInstanceCase() {
  class TestUnusedClassC4 {}
  let type = Type.from<TestUnusedClassC4>() as ClassType;
  let init = RuntimeDebug.countInstancesOfClass(type);
  keepAlive.push(new TestUnusedClassC4());
  let count = RuntimeDebug.countInstancesOfClass(type);
  arktest.assertLE(1, count);
  arktest.assertLE(count, init + 1);
}

function testMultipleClassIsolation() {
  class IsolatedA {}
  class IsolatedB {}
  let typeA = Type.from<IsolatedA>() as ClassType;
  let typeB = Type.from<IsolatedB>() as ClassType;
  let initA = RuntimeDebug.countInstancesOfClass(typeA);
  let initB = RuntimeDebug.countInstancesOfClass(typeB);
  keepAlive.push(new IsolatedA());
  keepAlive.push(new IsolatedA());
  keepAlive.push(new IsolatedB());
  let countA = RuntimeDebug.countInstancesOfClass(typeA);
  arktest.assertLE(2, countA);
  arktest.assertLE(countA, initA + 2);
  let countB = RuntimeDebug.countInstancesOfClass(typeB);
  arktest.assertLE(1, countB);
  arktest.assertLE(countB, initB + 1);
}

function testNestedObjectCount() {
  class Inner {}
  class Outer {
    field: Inner;
    constructor() {
      this.field = new Inner();
    }
  }
  let type = Type.from<Inner>() as ClassType;
  let init = RuntimeDebug.countInstancesOfClass(type);
  keepAlive.push(new Outer());
  keepAlive.push(new Outer());
  let count = RuntimeDebug.countInstancesOfClass(type);
  arktest.assertLE(2, count);
  arktest.assertLE(count, init + 2);
}

function testGenericClassCount() {
  class Box<T> {
    value: T;
    constructor(v: T) {
      this.value = v;
    }
  }
  let type = Type.from<Box<string>>() as ClassType;
  let init = RuntimeDebug.countInstancesOfClass(type);
  keepAlive.push(new Box<string>('a'));
  keepAlive.push(new Box<string>('b'));
  let count = RuntimeDebug.countInstancesOfClass(type);
  arktest.assertLE(2, count);
  arktest.assertLE(count, init + 2);
}

function testAnonymousClassCount() {
  class NamedAnonymousLike {}
  let type = Type.from<NamedAnonymousLike>() as ClassType;
  let init = RuntimeDebug.countInstancesOfClass(type);
  keepAlive.push(new NamedAnonymousLike());
  keepAlive.push(new NamedAnonymousLike());
  let count = RuntimeDebug.countInstancesOfClass(type);
  arktest.assertLE(2, count);
  arktest.assertLE(count, init + 2);
}

function main(): int {
  const suite = new arktest.ArkTestsuite('RuntimeDebug: countInstancesOfClass (Isolated)');
  suite.addTest('Basic', testCountInstancesOfClass);
  suite.addTest('Inheritance: Base only', testInheritanceBaseClassCount);
  suite.addTest('Inheritance: Subclass only', testInheritanceSubClassCount);
  suite.addTest('Zero-instance', testZeroInstanceCase);
  suite.addTest('Multiple classes: IsolatedA vs IsolatedB', testMultipleClassIsolation);
  suite.addTest('Nested object field', testNestedObjectCount);
  suite.addTest('Generic type: Box<string>', testGenericClassCount);
  suite.addTest('Anonymous class pattern', testAnonymousClassCount);
  keepAlive = [];
  GC.startGC(GC.Cause.FULL);
  return suite.run();
}
