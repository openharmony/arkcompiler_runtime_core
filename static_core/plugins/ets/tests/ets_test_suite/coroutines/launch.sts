/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

class MyPayload {
  public s: string;
  constructor(a: number) {
    this.s = `MyPayload.s=${a}`;
  }
}

const l0 = () => `l0`;
const l1 = (a0: string) => `l1: ${a0}`;
const l2 = (a0: string, a1: number, a2: MyPayload) => `l2: ${a0} ${a1} ${a2.s}`;

function f0() {
  return `f0`;
}

function f1(a0: string) {
  return `f1: ${a0}`;
}

function f2(a0: string, a1: number, a2: MyPayload) {
  return `f2: ${a0} ${a1} ${a2.s}`;
}

class C {
  static s0() {
    return `s0`;
  }

  static s1(a0: string) {
    return `s1: ${a0}`;
  }

  static s2(a0: string, a1: number, a2: MyPayload) {
    return `s2: ${a0} ${a1} ${a2.s}`;
  }
}

function main(): void {
  const l0Result = mlaunch<string, () => string>(
    l0
  ).Await();

  const l1Result = mlaunch<string, (a0: string) => string>(
    l1,
    "l1_arg_0"
  ).Await();

  const l2ResultJob: Job<string> = mlaunch<
    string,
    (a0: string, a1: number, a2: MyPayload) => string
  >(l2, "l2_arg_0", 2.73, new MyPayload(3.14));
  const l2Result = l2ResultJob.Await();

  assertEQ(l0Result, "l0");
  assertEQ(l1Result, "l1: l1_arg_0");
  assertEQ(l2Result, "l2: l2_arg_0 2.73 MyPayload.s=3.14");

  const f0ResultJob: Job<string> = mlaunch<string, () => string>(
    f0
  );
  const f0Result = f0ResultJob.Await();

  const f1ResultJob: Job<string> = mlaunch<string, (a0: string) => string>(
    f1,
    "f1_arg_0"
  );
  const f1Result = f1ResultJob.Await();

  const f2ResultJob: Job<string> = mlaunch<
    string,
    (a0: string, a1: number, a2: MyPayload) => string
  >(f2, "f2_arg_0", 2.73, new MyPayload(3.14));
  const f2Result = f2ResultJob.Await();

  assertEQ(f0Result, "f0");
  assertEQ(f1Result, "f1: f1_arg_0");
  assertEQ(f2Result, "f2: f2_arg_0 2.73 MyPayload.s=3.14");

  const s0Result = mlaunch<string, () => string>(
    C.s0
  ).Await();

  const s1Result = mlaunch<string, (a0: string) => string>(
    C.s1,
    "C.s1_arg_0"
  ).Await();

  const s2ResultJob = mlaunch<
    string,
    (a0: string, a1: number, a2: MyPayload) => string
  >(C.s2, "C.s2_arg_0", 2.73, new MyPayload(3.14));
  const s2Result = s2ResultJob.Await();

  assertEQ(s0Result, "s0");
  assertEQ(s1Result, "s1: C.s1_arg_0");
  assertEQ(s2Result, "s2: C.s2_arg_0 2.73 MyPayload.s=3.14");
}
