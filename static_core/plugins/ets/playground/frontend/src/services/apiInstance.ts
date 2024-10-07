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

import axios, { AxiosError, AxiosResponse, AxiosRequestConfig, AxiosHeaders, AxiosInstance } from 'axios';

interface CustomAxiosRequestConfig extends AxiosRequestConfig {
    _retry?: boolean;
    headers: AxiosHeaders;
}

let isRefreshing = false;
let failedQueue: Array<{ resolve: (token: string) => void; reject: (error: AxiosError) => void }> = [];

const processQueue = (error: AxiosError | null, token: string | null = null): void => {
    failedQueue.forEach(prom => {
        if (error) {
            prom.reject(error);
        } else {
            prom.resolve(token as string);
        }
    });
    failedQueue = [];
};

const handleResponseSuccess = (response: AxiosResponse): AxiosResponse => response;

const handleResponseError = async (err: AxiosError): Promise<CustomAxiosRequestConfig> => {
    const config = err.config as CustomAxiosRequestConfig;
    return Promise.reject(err);
};

export const getApiInstance = (baseURL = window?.runEnv?.apiUrl || window?.location?.origin): AxiosInstance => {
    const instance = axios.create();
    instance.defaults.baseURL = baseURL;

    /* Response interceptors - log errors */
    instance.interceptors.response.use(
        handleResponseSuccess,
        handleResponseError,
    );

    return instance;
};
