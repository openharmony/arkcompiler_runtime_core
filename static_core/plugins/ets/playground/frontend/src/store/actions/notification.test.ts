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
import notificationsReducer from '../slices/notifications';
import { showMessage, getMessageId } from './notification';

type TestState = { notifications: ReturnType<typeof notificationsReducer> };

const createTestStore = (): ReturnType<typeof configureStore<TestState>> => configureStore({
    reducer: {
        notifications: notificationsReducer,
    },
    middleware: (getDefaultMiddleware) => getDefaultMiddleware({
        serializableCheck: false,
    }),
});

describe('notification actions', () => {
    beforeEach(() => {
        jest.useFakeTimers();
    });

    afterEach(() => {
        jest.useRealTimers();
    });

    describe('getMessageId', () => {
        it('should return a timestamp-based number', () => {
            const id = getMessageId();
            expect(typeof id).toBe('number');
            expect(id).toBeGreaterThan(0);
        });
    });

    describe('showMessage', () => {
        it('should add a message to the store', async () => {
            const store = createTestStore();
            await store.dispatch(showMessage({
                type: 'success',
                title: 'Test',
                message: 'Hello',
                id: 123,
            }));

            const state = store.getState();
            expect(state.notifications.messages).toHaveLength(1);
            expect(state.notifications.messages[0]).toEqual({
                type: 'success',
                title: 'Test',
                message: 'Hello',
                id: 123,
            });
        });

        it('should auto-remove message after duration', async () => {
            const store = createTestStore();
            await store.dispatch(showMessage({
                type: 'info',
                title: 'Auto',
                message: 'Will disappear',
                id: 456,
                duration: 1000,
            }));

            expect(store.getState().notifications.messages).toHaveLength(1);

            jest.advanceTimersByTime(1000);

            expect(store.getState().notifications.messages).toHaveLength(0);
        });

        it('should use default duration when not specified', async () => {
            const store = createTestStore();
            await store.dispatch(showMessage({
                type: 'error',
                title: 'Default',
                message: 'Default duration',
                id: 789,
            }));

            expect(store.getState().notifications.messages).toHaveLength(1);

            jest.advanceTimersByTime(3000);

            expect(store.getState().notifications.messages).toHaveLength(0);
        });

        it('should generate id if not provided', async () => {
            const store = createTestStore();
            await store.dispatch(showMessage({
                type: 'success',
                title: 'No ID',
                message: 'Auto ID',
            }));

            const state = store.getState();
            expect(state.notifications.messages).toHaveLength(1);
            expect(state.notifications.messages[0].id).toBeDefined();
            expect(typeof state.notifications.messages[0].id).toBe('number');
        });
    });
});
