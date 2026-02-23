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

import { configureStore } from '@reduxjs/toolkit';
import featuresReducer from '../slices/features';
import { fetchFeatures } from './features';
import { featuresService } from '../../services/features';

jest.mock('../../services/features');

type TestState = { features: ReturnType<typeof featuresReducer> };

const createTestStore = (): ReturnType<typeof configureStore<TestState>> => configureStore({
    reducer: {
        features: featuresReducer,
    },
    middleware: (getDefaultMiddleware) => getDefaultMiddleware({
        serializableCheck: false,
    }),
});

describe('fetchFeatures thunk', () => {
    beforeEach(() => {
        jest.clearAllMocks();
    });

    it('should fetch features and set astMode on success', async () => {
        (featuresService.fetchFeatures as jest.Mock).mockResolvedValue({
            data: { astMode: 'manual' },
            error: '',
        });

        const store = createTestStore();
        await store.dispatch(fetchFeatures());

        const state = store.getState();
        expect(state.features.astMode).toBe('manual');
    });

    it('should set astMode to undefined when data exists but astMode is missing', async () => {
        (featuresService.fetchFeatures as jest.Mock).mockResolvedValue({
            data: {},
            error: '',
        });

        const store = createTestStore();
        await store.dispatch(fetchFeatures());

        const state = store.getState();
        expect(state.features.astMode).toBeUndefined();
    });

    it('should not update astMode on error', async () => {
        (featuresService.fetchFeatures as jest.Mock).mockResolvedValue({
            data: null,
            error: 'Failed',
        });

        const store = createTestStore();
        await store.dispatch(fetchFeatures());

        const state = store.getState();
        expect(state.features.astMode).toBe('auto');
    });
});
