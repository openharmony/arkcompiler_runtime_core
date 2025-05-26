/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

import reducer, { setTheme, setPrimaryColor, setDisasm, Theme, setRuntimeVerify } from './appState';

describe('appStateSlice reducer', () => {
    const initialState = {
        theme: (localStorage.getItem('theme') as Theme) ||
        (window.matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light'),
        disasm: false,
        verifier: true,
        runtimeVerify: false,
        primaryColor: '#e32b49',
        versions: {
            arktsVersion: '',
            backendVersion: '',
            es2panda: '',
            frontend: '',
        },
        versionsLoading: false,
        clearLogsEachRun: true
    };

    it('should return the initial state when passed an empty action', () => {
        expect(reducer(undefined, { type: '' })).toEqual(initialState);
    });

    it('should handle setTheme action', () => {
        const previousState = { ...initialState, theme: 'light' as Theme, clearLogsEachRun: true };
        expect(reducer(previousState, setTheme('dark'))).toEqual({
            ...previousState,
            theme: 'dark',
            clearLogsEachRun: true,
        });
    });

    it('should handle setPrimaryColor action', () => {
        const previousState = { ...initialState, primaryColor: '#ffffff', clearLogsEachRun: true };
        const newColor = '#ff5733';
        expect(reducer(previousState, setPrimaryColor(newColor))).toEqual({
            ...previousState,
            primaryColor: newColor,
            clearLogsEachRun: true,
        });
    });

    it('should handle setDisasm action', () => {
        const previousState = { ...initialState, disasm: false, clearLogsEachRun: true };
        expect(reducer(previousState, setDisasm(true))).toEqual({
            ...previousState,
            disasm: true,
            clearLogsEachRun: true,
        });
    });
});
