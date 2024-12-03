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

import React from 'react';
import {fireEvent, render, screen} from '@testing-library/react';
import LogsView from './LogsView';
import {ELogType, ILog} from '../../models/logs';

const mockClearFilters = jest.fn();

const mockLogs: ILog[] = [
    { message: 'This is a log message', from: ELogType.RUN_OUT },
    { message: 'This is an error message', from: ELogType.RUN_ERR },
];

describe('LogsView component', () => {
    beforeEach(() => {
        jest.clearAllMocks();
    });

    it('renders log messages', () => {
        render(<LogsView logArr={mockLogs} clearFilters={mockClearFilters} />);

        expect(screen.getByText('[LOG]:')).toBeInTheDocument();
        expect(screen.getByText('[ERR]:')).toBeInTheDocument();
        expect(screen.getByText(mockLogs[0].message)).toBeInTheDocument();
        expect(screen.getByText(mockLogs[1].message)).toBeInTheDocument();
    });

    it('filters log messages based on input', () => {
        render(<LogsView logArr={mockLogs} clearFilters={mockClearFilters} />);

        const filterInput = screen.getByPlaceholderText('Filter');
        fireEvent.change(filterInput, { target: { value: 'error' } });

        expect(screen.queryByText(mockLogs[0].message)).not.toBeInTheDocument();
        expect(screen.getByText(mockLogs[1].message)).toBeInTheDocument();
    });

    it('resets log messages on empty input', () => {
        render(<LogsView logArr={mockLogs} clearFilters={mockClearFilters} />);

        const filterInput = screen.getByPlaceholderText('Filter');
        fireEvent.change(filterInput, { target: { value: 'log' } });
        expect(screen.getByText(mockLogs[0].message)).toBeInTheDocument();
        expect(screen.queryByText(mockLogs[1].message)).not.toBeInTheDocument();

        fireEvent.change(filterInput, { target: { value: '' } });
        expect(screen.getByText(mockLogs[0].message)).toBeInTheDocument();
        expect(screen.getByText(mockLogs[1].message)).toBeInTheDocument();
    });

    it('calls clearFilters when clear icon is clicked', () => {
        render(<LogsView logArr={mockLogs} clearFilters={mockClearFilters} />);

        const clearIcon = screen.getByTestId('clear-icon');
        fireEvent.click(clearIcon);

        expect(mockClearFilters).toHaveBeenCalled();
    });
});
