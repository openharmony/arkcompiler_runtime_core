/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
import { fetchGetEntity } from './common';
import {ISyntax, syntaxModel} from '../models/syntax';

export enum ESyntaxEndpoints {
    GET_SYNTAX = '/syntax',
}

export const syntaxService = {
    fetchGetSyntax: async (): Promise<IServiceResponse<ISyntax>> => {
        return fetchGetEntity<ISyntax, ISyntax>(
            ESyntaxEndpoints.GET_SYNTAX,
            {} as ISyntax,
            syntaxModel.fromApi,
        );
    },
};
