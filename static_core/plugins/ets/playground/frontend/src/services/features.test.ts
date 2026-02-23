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

import { featuresService } from './features';
import { fetchGetEntity } from './common';

jest.mock('./common', () => ({
    fetchGetEntity: jest.fn(),
}));

describe('featuresService', () => {
    beforeEach(() => {
        jest.clearAllMocks();
    });

    describe('fetchFeatures', () => {
        it('should call fetchGetEntity and transform response', async () => {
            (fetchGetEntity as jest.Mock).mockResolvedValue({
                data: { astMode: 'auto' },
                error: '',
            });

            const response = await featuresService.fetchFeatures();

            expect(fetchGetEntity).toHaveBeenCalledWith(
                '/features',
                null,
                expect.any(Function),
            );
            expect(response).toEqual({ data: { astMode: 'auto' }, error: '' });
        });

        it('should transform raw API response correctly', async () => {
            (fetchGetEntity as jest.Mock).mockImplementation(
                (_endpoint, _fallback, transform) => {
                    const transformed = transform({ ast_mode: 'manual' });
                    return Promise.resolve({ data: transformed, error: '' });
                }
            );

            const response = await featuresService.fetchFeatures();

            expect(response.data).toEqual({ astMode: 'manual' });
        });

        it('should return error response on failure', async () => {
            (fetchGetEntity as jest.Mock).mockResolvedValue({
                data: null,
                error: 'Request failed',
            });

            const response = await featuresService.fetchFeatures();

            expect(response).toEqual({ data: null, error: 'Request failed' });
        });
    });
});
