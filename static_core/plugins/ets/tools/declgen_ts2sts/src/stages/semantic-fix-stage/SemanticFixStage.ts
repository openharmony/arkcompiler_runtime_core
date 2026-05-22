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

import { RecheckStage } from '../recheck-stage/RecheckStage';
import type { IStage } from '../Stage';
import { StageList } from '../Stage';
import { OverloadFixStage } from './OverloadFixStage';
import { OverrideFixStage } from './OverrideFixStage';

export class SemanticFixStage extends StageList<IStage[]> {
  constructor() {
    const stages: IStage[] = [];
    stages.push(new OverloadFixStage());
    stages.push(new RecheckStage());
    stages.push(new OverrideFixStage());

    super(stages, 'SemanticFixStage');
  }
}
