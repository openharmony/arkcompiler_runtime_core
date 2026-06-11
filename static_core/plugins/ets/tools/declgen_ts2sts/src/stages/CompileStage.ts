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

import { Stage, StageContext } from './Stage';

/**
 * First stage in the pipeline. Drives `Compiler.compile()` so the rest of the
 * pipeline can rely on `ctx.compiler.program` / `typeChecker` being initialised.
 *
 * Moving the compile step into the pipeline keeps the whole flow inside
 * `Pipeline.run()` and is required for incremental affected-analysis stages
 * to live after the compile step.
 */
export class CompileStage extends Stage<undefined, undefined> {
  override readonly cacheVersion = '1';

  constructor() {
    super();
  }

  execute(ctx: StageContext<undefined>): StageContext<undefined> {
    const files = ctx.compiler.rootFileNames;
    for (const file of files) {
      ctx.transformTargetInfos.set(file, {
        fileName: file,
        affected: true,
      });
    }
    ctx.compiler.compile();
    return ctx;
  }
}
