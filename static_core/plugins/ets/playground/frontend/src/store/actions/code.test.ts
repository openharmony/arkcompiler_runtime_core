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
import codeReducer from '../slices/code';
import logsReducer from '../slices/logs';
import optionsReducer from '../slices/options';
import appStateReducer from '../slices/appState';
import notificationsReducer from '../slices/notifications';
import { fetchCompileCode, fetchRunCode, fetchCodeByUuid, setCodeAction } from './code';
import { codeService } from '../../services/code';
import { setActiveLogTab } from '../slices/appState';

jest.mock('../../services/code');
jest.mock('../../utils/shareUtils', () => ({
    encodeShareData: jest.fn(() => 'encoded'),
    copyToClipboard: jest.fn(() => Promise.resolve(true)),
    buildShareUrl: jest.fn(() => 'http://localhost/share'),
}));

type TestState = {
    code: ReturnType<typeof codeReducer>;
    logs: ReturnType<typeof logsReducer>;
    options: ReturnType<typeof optionsReducer>;
    appState: ReturnType<typeof appStateReducer>;
    notifications: ReturnType<typeof notificationsReducer>;
};

const createTestStore = (): ReturnType<typeof configureStore<TestState>> => configureStore({
    reducer: {
        code: codeReducer,
        logs: logsReducer,
        options: optionsReducer,
        appState: appStateReducer,
        notifications: notificationsReducer,
    },
    middleware: (getDefaultMiddleware) => getDefaultMiddleware({
        serializableCheck: false,
    }),
});

describe('code thunks', () => {
    beforeEach(() => {
        jest.clearAllMocks();
    });

    describe('fetchCompileCode', () => {
        it('should compile code and store result on success', async () => {
            const mockResult = {
                output: 'compiled',
                error: '',
                exitCode: 0,
                disasm: null,
                verifier: null,
                ir_dump: null,
            };
            (codeService.fetchCompile as jest.Mock).mockResolvedValue({
                data: mockResult,
                error: '',
            });

            const store = createTestStore();
            await store.dispatch(fetchCompileCode());

            const state = store.getState();
            expect(state.code.isCompileLoading).toBe(false);
            expect(state.code.compileRes).toEqual(mockResult);
        });

        it('should set loading false on compile error', async () => {
            (codeService.fetchCompile as jest.Mock).mockResolvedValue({
                data: null,
                error: 'Compilation failed',
            });
            const consoleSpy = jest.spyOn(console, 'error').mockImplementation();

            const store = createTestStore();
            await store.dispatch(fetchCompileCode());

            const state = store.getState();
            expect(state.code.isCompileLoading).toBe(false);
            expect(state.code.compileRes).toBeNull();

            consoleSpy.mockRestore();
        });
    });

    describe('fetchRunCode', () => {
        it('should run code and store result on success', async () => {
            const mockResult = {
                output: 'Hello, ArkTS!',
                error: '',
                exitCode: 0,
                disasm: null,
                verifier: null,
                ir_dump: null,
            };
            (codeService.fetchRun as jest.Mock).mockResolvedValue({
                data: mockResult,
                error: '',
            });

            const store = createTestStore();
            await store.dispatch(fetchRunCode());

            const state = store.getState();
            expect(state.code.isRunLoading).toBe(false);
            expect(state.code.runRes).toEqual(mockResult);
        });

        it('should switch to compilation tab when compile has non-zero exit code', async () => {
            const mockResult = {
                compile: { output: '', error: 'Syntax error', exit_code: 1 },
                run: { output: '', error: '', exit_code: 0 },
                disassembly: { output: '', code: '', error: '' },
                verifier: { output: '', error: '' },
            };
            (codeService.fetchRun as jest.Mock).mockResolvedValue({
                data: mockResult,
                error: '',
            });

            const store = createTestStore();
            store.dispatch(setActiveLogTab('runtime'));
            expect(store.getState().appState.activeLogTab).toBe('runtime');

            await store.dispatch(fetchRunCode());

            expect(store.getState().appState.activeLogTab).toBe('compilation');
        });

        it('should keep runtime tab when compile succeeds', async () => {
            const mockResult = {
                compile: { output: '', error: '', exit_code: 0 },
                run: { output: 'Hello, ArkTS!', error: '', exit_code: 0 },
                disassembly: { output: '', code: '', error: '' },
                verifier: { output: '', error: '' },
            };
            (codeService.fetchRun as jest.Mock).mockResolvedValue({
                data: mockResult,
                error: '',
            });

            const store = createTestStore();
            store.dispatch(setActiveLogTab('runtime'));

            await store.dispatch(fetchRunCode());

            expect(store.getState().appState.activeLogTab).toBe('runtime');
        });

        it('should keep runtime tab when compile has warning but exit code 0', async () => {
            const mockResult = {
                compile: { output: 'Warning W163154: Consider do not use the keyword', error: '', exit_code: 0 },
                run: { output: 'Hello, ArkTS!', error: '', exit_code: 0 },
                disassembly: { output: '', code: '', error: '' },
                verifier: { output: '', error: '' },
            };
            (codeService.fetchRun as jest.Mock).mockResolvedValue({
                data: mockResult,
                error: '',
            });

            const store = createTestStore();
            store.dispatch(setActiveLogTab('runtime'));

            await store.dispatch(fetchRunCode());

            expect(store.getState().appState.activeLogTab).toBe('runtime');
        });

        it('should switch to compilation tab when aot_compile has non-zero exit code', async () => {
            const mockResult = {
                compile: { output: '', error: '', exit_code: 0 },
                aot_compile: { output: '', error: 'AOT error', exit_code: 1 },
                run: { output: '', error: '', exit_code: 0 },
                disassembly: { output: '', code: '', error: '' },
                verifier: { output: '', error: '' },
            };
            (codeService.fetchRun as jest.Mock).mockResolvedValue({
                data: mockResult,
                error: '',
            });

            const store = createTestStore();
            store.dispatch(setActiveLogTab('runtime'));

            await store.dispatch(fetchRunCode());

            expect(store.getState().appState.activeLogTab).toBe('compilation');
        });

        it('should set loading false on run error', async () => {
            (codeService.fetchRun as jest.Mock).mockResolvedValue({
                data: null,
                error: 'Runtime error',
            });
            const consoleSpy = jest.spyOn(console, 'error').mockImplementation();

            const store = createTestStore();
            await store.dispatch(fetchRunCode());

            const state = store.getState();
            expect(state.code.isRunLoading).toBe(false);
            expect(state.code.runRes).toBeNull();

            consoleSpy.mockRestore();
        });
    });

    describe('fetchCodeByUuid', () => {
        it('should fetch shared code and set code and options', async () => {
            (codeService.fetchShareCode as jest.Mock).mockResolvedValue({
                data: { code: 'shared code', options: { '--opt-level': '2' } },
                error: '',
            });

            const store = createTestStore();
            await store.dispatch(fetchCodeByUuid('abc-123'));

            const state = store.getState();
            expect(state.code.code).toBe('shared code');
            expect(state.options.pickedOptions).toEqual({ '--opt-level': '2' });
        });

        it('should handle error when fetching by uuid', async () => {
            (codeService.fetchShareCode as jest.Mock).mockResolvedValue({
                data: null,
                error: 'Not found',
            });
            const consoleSpy = jest.spyOn(console, 'error').mockImplementation();

            const store = createTestStore();
            await store.dispatch(fetchCodeByUuid('bad-uuid'));

            const state = store.getState();
            expect(state.code.code).toBe('console.log("Hello, ArkTS!");');

            consoleSpy.mockRestore();
        });
    });

    describe('setCodeAction', () => {
        it('should set code in the store', () => {
            const store = createTestStore();
            store.dispatch(setCodeAction('new code'));

            expect(store.getState().code.code).toBe('new code');
        });
    });
});
