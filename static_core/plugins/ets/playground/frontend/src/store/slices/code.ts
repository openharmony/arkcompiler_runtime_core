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
import {ICodeReq} from '../../models/code';

interface IState {
    isRunLoading: boolean
    isCompileLoading: boolean
    code: string
    compileRes: ICodeReq | null
    runRes: ICodeReq | null
}

const initialState: IState = {
    isRunLoading: false,
    isCompileLoading: false,
    code: '',
    compileRes: null,
    runRes: null
};

const codeSlice = createSlice({
    name: 'codeState',
    initialState,
    reducers: {
        setRunLoading(state, action: PayloadAction<boolean>) {
            state.isRunLoading = action.payload;
        },
        setCompileLoading(state, action: PayloadAction<boolean>) {
            state.isCompileLoading = action.payload;
        },
        setCode(state, action: PayloadAction<string>) {
            state.code = action.payload;
        },
        setCompileRes(state, action: PayloadAction<ICodeReq | null>) {
            state.compileRes = action.payload;
        },
        setRunRes(state, action: PayloadAction<ICodeReq | null>) {
            state.runRes = action.payload;
        },
    }
});

export const {
    setRunLoading,
    setCompileLoading,
    setCode,
    setCompileRes,
    setRunRes
} = codeSlice.actions;

export default codeSlice.reducer;
