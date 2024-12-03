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

import {
    selectCompileOutLogs,
    selectCompileErrLogs,
    selectRunOutLogs,
    selectRunErrLogs,
    selectDisasmOutLogs,
    selectDisasmErrLogs,
    selectOutLogs,
    selectErrLogs,
} from './logs';
import { RootState } from '..';
import {mockAllState} from './appState.test';

describe('Log Selectors', () => {
    let mockState: RootState;

    beforeEach(() => {
        mockState = {...mockAllState,
            logs: {
                compileOut: [{ message: 'compile output log', isRead: true }],
                compileErr: [{ message: 'compile error log', isRead: false }],
                runOut: [{ message: 'run output log', isRead: true }],
                runErr: [{ message: 'run error log', isRead: false }],
                disasmOut: [{ message: 'disassembly output log', isRead: true }],
                disasmErr: [{ message: 'disassembly error log', isRead: false }],
                out: [{ message: 'general output log', isRead: true }],
                err: [{ message: 'general error log', isRead: false }],
            },
        };
    });

    it('should select compile output logs', () => {
        expect(selectCompileOutLogs(mockState))
            .toEqual([{ message: 'compile output log', isRead: true }]);
    });

    it('should select compile error logs', () => {
        expect(selectCompileErrLogs(mockState))
            .toEqual([{ message: 'compile error log', isRead: false }]);
    });

    it('should select run output logs', () => {
        expect(selectRunOutLogs(mockState))
            .toEqual([{ message: 'run output log', isRead: true }]);
    });

    it('should select run error logs', () => {
        expect(selectRunErrLogs(mockState))
            .toEqual([{ message: 'run error log', isRead: false }]);
    });

    it('should select disassembly output logs', () => {
        expect(selectDisasmOutLogs(mockState))
            .toEqual([{ message: 'disassembly output log', isRead: true }]);
    });

    it('should select disassembly error logs', () => {
        expect(selectDisasmErrLogs(mockState))
            .toEqual([{ message: 'disassembly error log', isRead: false }]);
    });

    it('should select general output logs', () => {
        expect(selectOutLogs(mockState))
            .toEqual([{ message: 'general output log', isRead: true }]);
    });

    it('should select general error logs', () => {
        expect(selectErrLogs(mockState))
            .toEqual([{ message: 'general error log', isRead: false }]);
    });
});
