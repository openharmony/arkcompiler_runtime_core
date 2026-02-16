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

import { versionService, EVersionsEndpoints } from './versions';
import { fetchGetEntity } from './common';
import { versionsModel } from '../models/versions';

jest.mock('./common', () => ({
    fetchGetEntity: jest.fn(),
}));

describe('versionService', () => {
    beforeEach(() => {
        jest.clearAllMocks();
    });

    describe('fetchVersions', () => {
        it('should call fetchGetEntity with correct parameters', async () => {
            const mockVersions = { es2panda: '1.0', ark: '1.0' };
            (fetchGetEntity as jest.Mock).mockResolvedValue({ data: mockVersions, error: '' });

            const response = await versionService.fetchVersions();

            expect(fetchGetEntity).toHaveBeenCalledWith(
                EVersionsEndpoints.VERSIONS,
                {},
                versionsModel.fromApi,
            );
            expect(response).toEqual({ data: mockVersions, error: '' });
        });

        it('should return error response on failure', async () => {
            (fetchGetEntity as jest.Mock).mockResolvedValue({ data: {}, error: 'Network error' });

            const response = await versionService.fetchVersions();

            expect(response).toEqual({ data: {}, error: 'Network error' });
        });
    });
});
