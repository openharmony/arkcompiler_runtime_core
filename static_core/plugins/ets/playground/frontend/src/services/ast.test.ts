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

import { astService, EAstEndpoints } from './ast';
import { fetchPostEntity } from './common';
import { astModel, IAstReq } from '../models/ast';

jest.mock('./common', () => ({
    fetchPostEntity: jest.fn(),
}));

describe('astService', () => {
    const mockPayload = { code: 'function main(): void {}', options: {} };
    const mockAstResponse: IAstReq = {
        ast: '{"type":"Program"}',
        output: '',
        error: '',
        exit_code: 0,
    };

    beforeEach(() => {
        jest.clearAllMocks();
    });

    describe('fetchAst', () => {
        it('should call fetchPostEntity with auto trigger by default', async () => {
            (fetchPostEntity as jest.Mock).mockResolvedValue({ data: mockAstResponse, error: '' });

            const response = await astService.fetchAst(mockPayload);

            expect(fetchPostEntity).toHaveBeenCalledWith(
                mockPayload,
                EAstEndpoints.AST,
                null,
                astModel.fromApi,
            );
            expect(response).toEqual({ data: mockAstResponse, error: '' });
        });

        it('should append ?manual=1 when trigger is manual', async () => {
            (fetchPostEntity as jest.Mock).mockResolvedValue({ data: mockAstResponse, error: '' });

            await astService.fetchAst(mockPayload, 'manual');

            expect(fetchPostEntity).toHaveBeenCalledWith(
                mockPayload,
                `${EAstEndpoints.AST}?manual=1`,
                null,
                astModel.fromApi,
            );
        });

        it('should return error response on failure', async () => {
            (fetchPostEntity as jest.Mock).mockResolvedValue({ data: null, error: 'Request failed' });

            const response = await astService.fetchAst(mockPayload);

            expect(response).toEqual({ data: null, error: 'Request failed' });
        });
    });

    describe('fetchAstAuto', () => {
        it('should call fetchAst with auto trigger', async () => {
            (fetchPostEntity as jest.Mock).mockResolvedValue({ data: mockAstResponse, error: '' });

            await astService.fetchAstAuto(mockPayload);

            expect(fetchPostEntity).toHaveBeenCalledWith(
                mockPayload,
                EAstEndpoints.AST,
                null,
                astModel.fromApi,
            );
        });
    });

    describe('fetchAstManual', () => {
        it('should call fetchAst with manual trigger', async () => {
            (fetchPostEntity as jest.Mock).mockResolvedValue({ data: mockAstResponse, error: '' });

            await astService.fetchAstManual(mockPayload);

            expect(fetchPostEntity).toHaveBeenCalledWith(
                mockPayload,
                `${EAstEndpoints.AST}?manual=1`,
                null,
                astModel.fromApi,
            );
        });
    });
});
