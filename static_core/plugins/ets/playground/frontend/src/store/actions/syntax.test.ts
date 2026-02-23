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
import syntaxReducer from '../slices/syntax';
import { fetchSyntax } from './syntax';
import { syntaxService } from '../../services/syntax';

jest.mock('../../services/syntax');

type TestState = { syntax: ReturnType<typeof syntaxReducer> };

const createTestStore = (): ReturnType<typeof configureStore<TestState>> => configureStore({
    reducer: {
        syntax: syntaxReducer,
    },
    middleware: (getDefaultMiddleware) => getDefaultMiddleware({
        serializableCheck: false,
    }),
});

describe('fetchSyntax thunk', () => {
    beforeEach(() => {
        jest.clearAllMocks();
    });

    it('should fetch syntax and store result on success', async () => {
        const mockSyntax = {
            tokenizer: { root: [['/regex/', 'type']] },
        };
        (syntaxService.fetchGetSyntax as jest.Mock).mockResolvedValue({
            data: mockSyntax,
            error: '',
        });

        const store = createTestStore();
        await store.dispatch(fetchSyntax());

        const state = store.getState();
        expect(state.syntax.isLoading).toBe(false);
        expect(state.syntax.syntax).toEqual(mockSyntax);
    });

    it('should set loading false and not store result on error', async () => {
        (syntaxService.fetchGetSyntax as jest.Mock).mockResolvedValue({
            data: null,
            error: 'Failed to fetch',
        });

        const store = createTestStore();
        await store.dispatch(fetchSyntax());

        const state = store.getState();
        expect(state.syntax.isLoading).toBe(false);
        expect(state.syntax.syntax).toEqual({ tokenizer: { root: [] } });
    });
});
