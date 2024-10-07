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

export type TOptionVal = string | number | boolean;

export interface IOption {
    flag: string
    values: TOptionVal[]
    default: TOptionVal
    isSelected?: TOptionVal
}
export type TData = {
    compileOptions: IOption[]
};

export type TObj = {
    [key: string]: number | string | boolean
};

export const optionsModel = {
    fromApi: (data: TData): IOption[] => (data?.compileOptions?.map(item => ({
            flag: item?.flag || '',
            values: item?.values || [],
            default: item?.default?.toString() || '',
            isSelected: item?.default?.toString() || '',
        })) || [])
};
