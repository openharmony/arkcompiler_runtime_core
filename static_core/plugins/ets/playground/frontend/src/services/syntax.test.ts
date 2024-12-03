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

import { syntaxService, ESyntaxEndpoints } from './syntax';
import { fetchGetEntity } from './common';
import { syntaxModel, ISyntax } from '../models/syntax';

jest.mock('./common', () => ({
    fetchGetEntity: jest.fn(),
}));

describe('syntaxService', () => {
    describe('fetchGetSyntax', () => {
        it('should call fetchGetEntity with correct parameters and return syntax on success', async () => {
            const mockSyntax: ISyntax = {
                tokenizer: { root: [['/regex/', 'type']] },
            };
            (fetchGetEntity as jest.Mock).mockResolvedValue({ data: mockSyntax, error: '' });

            const response = await syntaxService.fetchGetSyntax();

            expect(fetchGetEntity).toHaveBeenCalledWith(
                ESyntaxEndpoints.GET_SYNTAX,
                {} as ISyntax,
                syntaxModel.fromApi,
            );
            expect(response).toEqual({ data: mockSyntax, error: '' });
        });

        it('should return fallback value (empty syntax object) on failure', async () => {
            (fetchGetEntity as jest.Mock).mockResolvedValue({ data: {} as ISyntax, error: 'Request failed' });

            const response = await syntaxService.fetchGetSyntax();

            expect(response).toEqual({ data: {} as ISyntax, error: 'Request failed' });
        });
    });
});
