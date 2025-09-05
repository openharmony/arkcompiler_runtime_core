/*
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

import { IServiceResponse } from './types';
import { fetchPostEntity } from './common';
import { IAstFetch, IAstReq, astModel } from '../models/ast';

export enum EAstEndpoints {
  AST = '/ast',
}

type AstTrigger = 'auto' | 'manual';

export const astService = {
  fetchAst: async (
    payload: IAstFetch,
    trigger: AstTrigger = 'auto'
  ): Promise<IServiceResponse<IAstReq>> => {
    const endpoint =
      trigger === 'manual' ? `${EAstEndpoints.AST}?manual=1` : EAstEndpoints.AST;
    // @ts-ignore
    return fetchPostEntity<IAstFetch, IAstReq>(
      payload,
      endpoint,
      null,
      astModel.fromApi
    );
  },

  fetchAstAuto: (payload: IAstFetch) => astService.fetchAst(payload, 'auto'),
  fetchAstManual: (payload: IAstFetch) => astService.fetchAst(payload, 'manual'),
};
