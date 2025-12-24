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

import { IObj } from '../store/slices/options';

export interface IAstFetch {
    code: string;
    options?: IObj | null;
}

export interface IAstReq {
    ast: string | null;
    output: string;
    error: string;
    exit_code: number;
}

export const astModel = {
    fromApi: (data: Partial<IAstReq>): IAstReq => ({
      ast: data?.ast ?? null,
      output: data?.output ?? '',
      error: data?.error ?? '',
      exit_code: typeof data?.exit_code === 'number' ? data.exit_code : 0,
    }),
};
