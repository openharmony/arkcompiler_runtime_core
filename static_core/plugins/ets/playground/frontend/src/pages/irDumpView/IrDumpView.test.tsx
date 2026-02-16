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

import React from 'react';
import { render, screen } from '@testing-library/react';
import { CompilerDumpView, DisasmDumpView } from './IrDumpView';
import { Provider } from 'react-redux';
import configureMockStore from 'redux-mock-store';
import { ThemeProvider } from '../../components/theme/ThemeContext';
import { AppDispatch } from '../../store';

const mockStore = configureMockStore<AppDispatch>();

describe('IrDumpView components', () => {
    const renderWithProviders = (component: JSX.Element, storeOverrides = {}): ReturnType<typeof render> => {
        const store = mockStore({
            // @ts-ignore
            appState: { theme: 'light', primaryColor: '#e32b49' },
            code: {
                isCompileLoading: false,
                isRunLoading: false,
                compileRes: null,
                runRes: null,
                code: '',
            },
            ...storeOverrides,
        });
        return render(
            <Provider store={store}>
                <ThemeProvider>{component}</ThemeProvider>
            </Provider>
        );
    };

    describe('CompilerDumpView', () => {
        it('renders the editor component', () => {
            renderWithProviders(<CompilerDumpView />);
            expect(screen.getByTestId('editor')).toBeInTheDocument();
        });

        it('shows Loading... when compile is loading', () => {
            renderWithProviders(<CompilerDumpView />, {
                code: {
                    isCompileLoading: true,
                    isRunLoading: false,
                    compileRes: null,
                    runRes: null,
                    code: '',
                },
            });
            expect(screen.getByTestId('editor').textContent).toContain('Loading...');
        });

        it('shows Loading... when run is loading', () => {
            renderWithProviders(<CompilerDumpView />, {
                code: {
                    isCompileLoading: false,
                    isRunLoading: true,
                    compileRes: null,
                    runRes: null,
                    code: '',
                },
            });
            expect(screen.getByTestId('editor').textContent).toContain('Loading...');
        });

        it('displays compiler dump content from compile result', () => {
            renderWithProviders(<CompilerDumpView />, {
                code: {
                    isCompileLoading: false,
                    isRunLoading: false,
                    compileRes: {
                        ir_dump: {
                            compiler_dump: [
                                { name: 'pass1.ir', content: 'IR content here' },
                            ],
                            disasm_dump: null,
                        },
                    },
                    runRes: null,
                    code: '',
                },
            });
            expect(screen.getByTestId('editor').textContent).toContain('pass1.ir');
            expect(screen.getByTestId('editor').textContent).toContain('IR content here');
        });

        it('displays multiple compiler dump files', () => {
            renderWithProviders(<CompilerDumpView />, {
                code: {
                    isCompileLoading: false,
                    isRunLoading: false,
                    compileRes: {
                        ir_dump: {
                            compiler_dump: [
                                { name: 'file1.ir', content: 'content1' },
                                { name: 'file2.ir', content: 'content2' },
                            ],
                            disasm_dump: null,
                        },
                    },
                    runRes: null,
                    code: '',
                },
            });
            const editorContent = screen.getByTestId('editor').textContent;
            expect(editorContent).toContain('file1.ir');
            expect(editorContent).toContain('content1');
            expect(editorContent).toContain('file2.ir');
            expect(editorContent).toContain('content2');
        });

        it('shows empty when no dump data', () => {
            renderWithProviders(<CompilerDumpView />, {
                code: {
                    isCompileLoading: false,
                    isRunLoading: false,
                    compileRes: null,
                    runRes: null,
                    code: '',
                },
            });
            expect(screen.getByTestId('editor')).toBeInTheDocument();
        });
    });

    describe('DisasmDumpView', () => {
        it('renders the editor component', () => {
            renderWithProviders(<DisasmDumpView />);
            expect(screen.getByTestId('editor')).toBeInTheDocument();
        });

        it('displays disasm dump content from compile result', () => {
            renderWithProviders(<DisasmDumpView />, {
                code: {
                    isCompileLoading: false,
                    isRunLoading: false,
                    compileRes: {
                        ir_dump: {
                            compiler_dump: null,
                            disasm_dump: 'Disassembly output text',
                        },
                    },
                    runRes: null,
                    code: '',
                },
            });
            expect(screen.getByTestId('editor').textContent).toContain('Disassembly output text');
        });

        it('falls back to run result if compile result is null', () => {
            renderWithProviders(<DisasmDumpView />, {
                code: {
                    isCompileLoading: false,
                    isRunLoading: false,
                    compileRes: null,
                    runRes: {
                        ir_dump: {
                            compiler_dump: null,
                            disasm_dump: 'Run disasm output',
                        },
                    },
                    code: '',
                },
            });
            expect(screen.getByTestId('editor').textContent).toContain('Run disasm output');
        });

        it('applies dark theme', () => {
            localStorage.setItem('theme', 'dark');
            renderWithProviders(<DisasmDumpView />);
            expect(screen.getByTestId('editor')).toHaveAttribute('data-theme', 'vs-dark');
        });

        it('applies light theme', () => {
            localStorage.setItem('theme', 'light');
            renderWithProviders(<DisasmDumpView />);
            expect(screen.getByTestId('editor')).toHaveAttribute('data-theme', 'light');
        });
    });
});
