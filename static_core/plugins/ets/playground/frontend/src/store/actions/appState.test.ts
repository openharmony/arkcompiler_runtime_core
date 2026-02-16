/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
import { store, RootState } from '../index';
import { setTheme } from '../slices/appState';
import { setOptionsLoading } from '../slices/options';
import { setSyntaxLoading } from '../slices/syntax';
import appStateReducer from '../slices/appState';
import { fetchVersions } from './appState';
import { versionService } from '../../services/versions';

jest.mock('../../services/versions');

describe('Redux store structure and functionality', () => {
    it('should initialize with the correct default state', () => {
        const initialState: RootState = store.getState();
        expect(initialState.appState.theme).toBe('light'); // Assuming default as 'light'
        expect(initialState.options.isLoading).toBe(false);
        expect(initialState.syntax.isLoading).toBe(false);
    });

    it('should handle appState actions correctly', () => {
        store.dispatch(setTheme('dark'));
        const state = store.getState();
        expect(state.appState.theme).toBe('dark');
    });

    it('should handle options loading state change', () => {
        store.dispatch(setOptionsLoading(true));
        const state = store.getState();
        expect(state.options.isLoading).toBe(true);
    });

    it('should handle syntax loading state change', () => {
        store.dispatch(setSyntaxLoading(true));
        const state = store.getState();
        expect(state.syntax.isLoading).toBe(true);
    });
});

type TestState = { appState: ReturnType<typeof appStateReducer> };

const createTestStore = (): ReturnType<typeof configureStore<TestState>> => configureStore({
    reducer: {
        appState: appStateReducer,
    },
    middleware: (getDefaultMiddleware) => getDefaultMiddleware({
        serializableCheck: false,
    }),
});

describe('fetchVersions thunk', () => {
    beforeEach(() => {
        jest.clearAllMocks();
    });

    it('should fetch versions and store result on success', async () => {
        const mockVersions = { es2panda: '1.0.0', ark: '2.0.0' };
        (versionService.fetchVersions as jest.Mock).mockResolvedValue({
            data: mockVersions,
            error: '',
        });

        const testStore = createTestStore();
        await testStore.dispatch(fetchVersions());

        const state = testStore.getState();
        expect(state.appState.versionsLoading).toBe(false);
        expect(state.appState.versions).toEqual(mockVersions);
    });

    it('should set loading false on error response', async () => {
        (versionService.fetchVersions as jest.Mock).mockResolvedValue({
            data: null,
            error: 'API error',
        });
        const consoleSpy = jest.spyOn(console, 'error').mockImplementation();

        const testStore = createTestStore();
        await testStore.dispatch(fetchVersions());

        const state = testStore.getState();
        expect(state.appState.versionsLoading).toBe(false);

        consoleSpy.mockRestore();
    });

    it('should set loading false on exception', async () => {
        (versionService.fetchVersions as jest.Mock).mockRejectedValue(
            new Error('Network failure')
        );
        const consoleSpy = jest.spyOn(console, 'error').mockImplementation();

        const testStore = createTestStore();
        await testStore.dispatch(fetchVersions());

        const state = testStore.getState();
        expect(state.appState.versionsLoading).toBe(false);

        consoleSpy.mockRestore();
    });
});
