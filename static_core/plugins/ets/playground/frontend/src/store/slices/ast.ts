/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
import { IAstReq } from '../../models/ast';

interface IAstState {
  isAstLoading: boolean;
  astRes: IAstReq | null;
}

const initialState: IAstState = {
  isAstLoading: false,
  astRes: null,
};

const astSlice = createSlice({
  name: 'ast',
  initialState,
  reducers: {
    setAstLoading(state, action: PayloadAction<boolean>) {
      state.isAstLoading = action.payload;
    },
    setAstRes(state, action: PayloadAction<IAstReq | null>) {
      state.astRes = action.payload;
    },
  },
});

export const {
    setAstLoading,
    setAstRes
} = astSlice.actions;

export default astSlice.reducer;
