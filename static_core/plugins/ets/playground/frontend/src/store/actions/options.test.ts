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
import optionsReducer from '../slices/options';
import { fetchOptions, pickOptions } from './options';
import { optionsService } from '../../services/options';

jest.mock('../../services/options');

type TestState = { options: ReturnType<typeof optionsReducer> };

const createTestStore = (): ReturnType<typeof configureStore<TestState>> => configureStore({
    reducer: {
        options: optionsReducer,
    },
    middleware: (getDefaultMiddleware) => getDefaultMiddleware({
        serializableCheck: false,
    }),
});

describe('fetchOptions thunk', () => {
    beforeEach(() => {
        jest.clearAllMocks();
    });

    it('should fetch options and store result on success', async () => {
        const mockOptions = [
            { flag: '--opt-level', isSelected: '2', values: ['0', '1', '2'] },
            { flag: '--debug-info', isSelected: 'false', values: ['true', 'false'] },
        ];
        (optionsService.fetchGetOptions as jest.Mock).mockResolvedValue({
            data: mockOptions,
            error: '',
        });

        const store = createTestStore();
        await store.dispatch(fetchOptions());

        const state = store.getState();
        expect(state.options.isLoading).toBe(false);
        expect(state.options.options).toEqual(mockOptions);
        expect(state.options.pickedOptions).toEqual({
            '--opt-level': '2',
            '--debug-info': 'false',
        });
    });

    it('should not overwrite existing picked options', async () => {
        const mockOptions = [
            { flag: '--opt-level', isSelected: '2', values: ['0', '1', '2'] },
        ];
        (optionsService.fetchGetOptions as jest.Mock).mockResolvedValue({
            data: mockOptions,
            error: '',
        });

        const store = createTestStore();
        store.dispatch(pickOptions({ '--opt-level': '0' }));

        await store.dispatch(fetchOptions());

        const state = store.getState();
        expect(state.options.pickedOptions).toEqual({ '--opt-level': '0' });
    });

    it('should set loading false on error', async () => {
        (optionsService.fetchGetOptions as jest.Mock).mockResolvedValue({
            data: null,
            error: 'Network error',
        });

        const store = createTestStore();
        await store.dispatch(fetchOptions());

        const state = store.getState();
        expect(state.options.isLoading).toBe(false);
        expect(state.options.options).toBeNull();
    });
});
