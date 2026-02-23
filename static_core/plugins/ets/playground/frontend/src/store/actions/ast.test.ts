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
import astReducer from '../slices/ast';
import logsReducer from '../slices/logs';
import codeReducer from '../slices/code';
import optionsReducer from '../slices/options';
import appStateReducer from '../slices/appState';
import featuresReducer from '../slices/features';
import { fetchAst } from './ast';
import { astService } from '../../services/ast';

jest.mock('../../services/ast');

type TestState = {
    ast: ReturnType<typeof astReducer>;
    logs: ReturnType<typeof logsReducer>;
    code: ReturnType<typeof codeReducer>;
    options: ReturnType<typeof optionsReducer>;
    appState: ReturnType<typeof appStateReducer>;
    features: ReturnType<typeof featuresReducer>;
};

const createTestStore = (): ReturnType<typeof configureStore<TestState>> => configureStore({
    reducer: {
        ast: astReducer,
        logs: logsReducer,
        code: codeReducer,
        options: optionsReducer,
        appState: appStateReducer,
        features: featuresReducer,
    },
    middleware: (getDefaultMiddleware) => getDefaultMiddleware({
        serializableCheck: false,
    }),
});

describe('fetchAst thunk', () => {
    beforeEach(() => {
        jest.clearAllMocks();
    });

    it('should set loading, fetch AST, and store result on success', async () => {
        const mockAstData = {
            ast: '{"type":"Program"}',
            output: 'parsed ok',
            error: '',
            exit_code: 0,
        };
        (astService.fetchAst as jest.Mock).mockResolvedValue({
            data: mockAstData,
            error: '',
        });

        const store = createTestStore();
        await store.dispatch(fetchAst(undefined));

        const state = store.getState();
        expect(state.ast.isAstLoading).toBe(false);
        expect(state.ast.astRes).toEqual(mockAstData);
    });

    it('should set loading false and not store result on error', async () => {
        (astService.fetchAst as jest.Mock).mockResolvedValue({
            data: null,
            error: 'Parse error',
        });
        const consoleSpy = jest.spyOn(console, 'error').mockImplementation();

        const store = createTestStore();
        await store.dispatch(fetchAst(undefined));

        const state = store.getState();
        expect(state.ast.isAstLoading).toBe(false);
        expect(state.ast.astRes).toBeNull();

        consoleSpy.mockRestore();
    });

    it('should use manual trigger when passed', async () => {
        (astService.fetchAst as jest.Mock).mockResolvedValue({
            data: { ast: null, output: '', error: '', exit_code: 0 },
            error: '',
        });

        const store = createTestStore();
        await store.dispatch(fetchAst('manual'));

        expect(astService.fetchAst).toHaveBeenCalledWith(
            expect.objectContaining({ code: expect.any(String) }),
            'manual',
        );
    });

    it('should clear logs when clearLogsEachRun is true', async () => {
        (astService.fetchAst as jest.Mock).mockResolvedValue({
            data: { ast: null, output: '', error: '', exit_code: 0 },
            error: '',
        });

        const store = createTestStore();
        await store.dispatch(fetchAst(undefined));

        const state = store.getState();
        expect(state.logs.out).toEqual([]);
        expect(state.logs.err).toEqual([]);
    });
});
