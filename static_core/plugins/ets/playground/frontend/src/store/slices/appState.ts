/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

import { createSlice, PayloadAction } from '@reduxjs/toolkit';

export type Theme = 'light' | 'dark';
interface IState {
    theme: Theme;
    primaryColor: string;
    disasm: boolean
}

const initialState: IState = {
    theme: (localStorage.getItem('theme') as Theme) || (window.matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light'),
    disasm: false,
    primaryColor: '#e32b49'
};

const appStateSlice = createSlice({
    name: 'appState',
    initialState,
    reducers: {
        setTheme: (state, action: PayloadAction<'light' | 'dark'>) => {
            state.theme = action.payload;
        },
        setPrimaryColor: (state, action: PayloadAction<string>) => {
            state.primaryColor = action.payload;
        },
        setDisasm: (state, action: PayloadAction<boolean>) => {
            state.disasm = action.payload;
        },
    },
});

export const {
    setTheme,
    setDisasm,
    setPrimaryColor,
} = appStateSlice.actions;

export default appStateSlice.reducer;
