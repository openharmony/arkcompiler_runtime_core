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

import { IStage, StageContext } from './stages/Stage';

type FirstInput<S extends readonly IStage<unknown, unknown>[]> = S[0] extends IStage<infer I, unknown> ? I : never;

type LastOutput<S extends readonly IStage<unknown, unknown>[]> = S extends readonly [...unknown[], infer Last]
  ? Last extends IStage<unknown, infer O>
    ? O
    : never
  : never;

export class Pipeline<Stages extends readonly IStage<unknown, unknown>[]> {
  private stages: IStage<unknown, unknown>[];

  constructor(stages?: Stages) {
    this.stages = stages ? [...stages] : [];
  }

  run(initial: StageContext<FirstInput<Stages>>): StageContext<LastOutput<Stages>> {
    let ctx: StageContext<unknown> = initial;
    for (const stage of this.stages) {
      ctx = stage.run(ctx);
    }
    return ctx as StageContext<LastOutput<Stages>>;
  }

  add(stage: IStage<unknown, unknown>): this {
    this.stages.push(stage);
    return this;
  }

  insertBefore(index: number, stage: IStage<unknown, unknown>): this {
    if (index < 0 || index > this.stages.length) {
      throw new RangeError(`Pipeline.insertBefore: index ${index} is out of range [0, ${this.stages.length}]`);
    }
    this.stages.splice(index, 0, stage);
    return this;
  }

  insertAfter(index: number, stage: IStage<unknown, unknown>): this {
    if (index < 0 || index >= this.stages.length) {
      throw new RangeError(`Pipeline.insertAfter: index ${index} is out of range [0, ${this.stages.length - 1}]`);
    }
    this.stages.splice(index + 1, 0, stage);
    return this;
  }

  get size(): number {
    return this.stages.length;
  }
}
