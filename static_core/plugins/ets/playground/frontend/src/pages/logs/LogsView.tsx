/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

import React, {useEffect, useRef, useState} from 'react';
import {ILog} from '../../models/logs';
import styles from './styles.module.scss';
import {Icon, Checkbox} from '@blueprintjs/core';
import cx from 'classnames';
import { AppDispatch } from '../../store';
import { useDispatch, useSelector } from 'react-redux';
import {
    setOutLogs,
    setErrLogs,
    setJumpTo
} from '../../store/slices/logs';
import {
    selectOutLogs,
    selectErrLogs
} from '../../store/selectors/logs';


interface IProps {
    logArr: ILog[]
    clearFilters: () => void
    logType: 'out' | 'err' | 'compilation' | 'runtime';
}

const LogsView = ({logArr, clearFilters, logType}: IProps): JSX.Element => {
    const [logs, setLogs] = useState<ILog[]>(logArr);
    const [showOut, setShowOut] = useState<boolean>(true);
    const [showErr, setShowErr] = useState<boolean>(true);
    const [showCompile, setShowCompile] = useState<boolean>(true);
    const [showAot, setShowAot] = useState<boolean>(true);
    const [showDisasm, setShowDisasm] = useState<boolean>(true);
    const [showVerifier, setShowVerifier] = useState<boolean>(true);
    const [filterText, setFilterText] = useState<string>('');
    const dispatch = useDispatch<AppDispatch>();
    const containerRef = useRef<HTMLDivElement | null>(null);

    const outLogs = useSelector(selectOutLogs);
    const errLogs = useSelector(selectErrLogs);

    useEffect(() => {
        let filtered = logArr;

        // Filter by type (out/err)
        filtered = filtered.filter(log => {
            const isErr = log.from?.includes('Err');
            if (!showOut && !isErr) {
                return false;
            }
            if (!showErr && isErr) {
                return false;
            }
            return true;
        });

        // Filter by source (Compile, AOT, Disasm, Verifier)
        filtered = filtered.filter(log => {
            const source = log.from || '';

            const isCompile = source.includes('compile') && !source.includes('aot');
            const isRun = (source === 'runOut' || source === 'runErr');
            const isAot = source.includes('aot');
            const isDisasm = source.includes('disasm');
            const isVerifier = source.includes('verifier');

            if ((isCompile || isRun) && !showCompile) {
                return false;
            }

            if (isAot && !showAot) {
                return false;
            }

            if (isDisasm && !showDisasm) {
                return false;
            }

            if (isVerifier && !showVerifier) {
                return false;
            }

            return true;
        });

        // Filter by text
        if (filterText) {
            filtered = filtered.filter(el =>
                el.message.some(m => m.message.includes(filterText))
            );
        }

        setLogs(filtered);
    }, [logArr, showOut, showErr, showCompile, showAot, showDisasm, showVerifier, filterText]);

    useEffect(() => {
        if (!containerRef.current) {
            return undefined;
        }
        const observer = new IntersectionObserver(
            (entries) => {
                entries.forEach((entry) => {
                    if (entry.isIntersecting) {
                        const logIndex = Number(entry.target.getAttribute('data-index'));

                        if (logIndex !== undefined && !logs[logIndex].isRead) {
                            const currentLog = logs[logIndex];
                            const updatedLogs = [...logs];
                            updatedLogs[logIndex] = { ...updatedLogs[logIndex], isRead: true };
                            setLogs(updatedLogs);

                            const isOutputLog = currentLog.message.some(m => m.source === 'output');

                            if (isOutputLog) {
                                const updatedOutLogs = outLogs.map(log =>
                                    log === currentLog ? { ...log, isRead: true } : log
                                );
                                dispatch(setOutLogs(updatedOutLogs));
                            } else {
                                const updatedErrLogs = errLogs.map(log =>
                                    log === currentLog ? { ...log, isRead: true } : log
                                );
                                dispatch(setErrLogs(updatedErrLogs));
                            }
                        }
                    }
                });
            },
            { threshold: 0.1 }
        );

        containerRef.current.querySelectorAll('[data-index]').forEach((el): void => observer.observe(el));

        return (): void => {
            observer.disconnect();
        };
    }, [logs, dispatch, outLogs, errLogs]);

    return (
        <div className={styles.container} data-testid={`logs-container-${logType}`}>
            <div className={styles.containerLogs} ref={containerRef} data-testid={`logs-content-${logType}`}>
                {logs.map((log: ILog, index) => (
                    <div key={index} data-index={index} className={styles.logRow} data-testid="log-row">
                        <span
                            className={cx({
                                [styles.tag]: true,
                                [styles.red]: log.from?.includes('Err')})}
                        >
                            [{log.from?.includes('Err') ? 'err' : 'out'}]
                        </span>
                        {log.from && !log.from.includes('aot') &&
                         (log.from.includes('compile') || log.from === 'runOut' || log.from === 'runErr') && (
                            <span className={cx(styles.tag, styles.modeTag)}>[cmpl]</span>
                        )}
                        {log.from?.includes('aot') && (
                            <span className={cx(styles.tag, styles.modeTag)}>[AOT]</span>
                        )}
                        <div className={styles.messageContainer}>
                            {log.message.map((el, ind) => (
                                <pre
                                    key={`${el.message}-${ind}`}
                                    className={styles.rowContainer}
                                >
                                    <span
                                        className={styles.logText}
                                        data-testid="log-message"
                                        onClick={(): void => {
                                            if (el?.line && el?.column) {
                                                dispatch(setJumpTo({line: el.line, column: el.column, nonce: Date.now()}));
                                            }
                                        }}
                                    >{el.message}</span>
                                </pre>
                            ))}
                        </div>
                    </div>
                ))}
            </div>
            <div className={styles.filterContainer}>
                <button
                    className={styles.clearIc}
                    onClick={clearFilters}
                    aria-label="Clear logs"
                    type="button"
                >
                    <Icon icon={'disable'} />
                </button>
                <Checkbox
                    checked={showOut}
                    label="Out"
                    onChange={(e): void => setShowOut(e.currentTarget.checked)}
                    className={styles.checkbox}
                />
                <Checkbox
                    checked={showErr}
                    label="Err"
                    onChange={(e): void => setShowErr(e.currentTarget.checked)}
                    className={styles.checkbox}
                />
                <Checkbox
                    checked={showCompile}
                    label="Compile"
                    onChange={(e): void => setShowCompile(e.currentTarget.checked)}
                    className={styles.checkbox}
                />
                <Checkbox
                    checked={showAot}
                    label="AOT"
                    onChange={(e): void => setShowAot(e.currentTarget.checked)}
                    className={styles.checkbox}
                />
                <Checkbox
                    checked={showDisasm}
                    label="Disasm"
                    onChange={(e): void => setShowDisasm(e.currentTarget.checked)}
                    className={styles.checkbox}
                />
                <Checkbox
                    checked={showVerifier}
                    label="Verifier"
                    onChange={(e): void => setShowVerifier(e.currentTarget.checked)}
                    className={styles.checkbox}
                />
                <input
                    placeholder="Filter"
                    className={styles.input}
                    onChange={(e): void => setFilterText(e.target.value)}
                    value={filterText}
                />
            </div>
        </div>);
};

export default LogsView;
