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

import React, {useEffect, useRef, useState} from 'react';
import {ILog} from '../../models/logs';
import styles from './styles.module.scss';
import {Icon} from '@blueprintjs/core';
import cx from 'classnames';
import { AppDispatch } from '../../store';
import { useDispatch } from 'react-redux';
import { setOutLogs, setErrLogs } from '../../store/slices/logs';


interface IProps {
    logArr: ILog[]
    clearFilters: () => void
    logType: 'out' | 'err';
}

const LogsView = ({logArr, clearFilters, logType}: IProps): JSX.Element => {
    const [logs, setLogs] = useState<ILog[]>(logArr);
    const dispatch = useDispatch<AppDispatch>();
    const containerRef = useRef<HTMLDivElement | null>(null);

    const handleFilter = (val: string): void => {
        if (!val) {
            setLogs(logArr);
            return;
        }
        const filtered = logs.filter(el => el.message.includes(val));
        setLogs(filtered);
    };
    useEffect(() => {
        setLogs(logArr);
    }, [logArr]);

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
                            const updatedLogs = [...logs];
                            updatedLogs[logIndex] = { ...updatedLogs[logIndex], isRead: true };
                            setLogs(updatedLogs);
                            if (logType === 'out') {
                                dispatch(setOutLogs(updatedLogs));
                            } else if (logType === 'err') {
                                dispatch(setErrLogs(updatedLogs));
                            }
                        }
                    }
                });
            },
            { threshold: 0.1 }
        );

        containerRef.current.querySelectorAll('[data-index]').forEach((el): void => observer.observe(el));

        return () => {
            observer.disconnect();
        };
    }, [logs, dispatch, logType]);

    return (
        <div className={styles.container}>
            <div className={styles.containerLogs} ref={containerRef}>
                {logs.map((log: ILog, index) => (
                    <pre key={index} data-index={index} className={styles.rowContainer}>
                        <span
                            className={cx({
                                [styles.tag]: true,
                                [styles.red]: log?.from?.includes('Err')})}
                        >
                            [{log?.from?.includes('Err') ? 'ERR' : 'LOG'}]:
                        </span>
                        <span className={styles.logText}>{log.message}</span>
                    </pre>
                ))}
            </div>
            <div className={styles.filterContainer}>
                <Icon
                    icon={'disable'}
                    className={styles.clearIc}
                    onClick={clearFilters}
                    data-testid="clear-icon"
                />
                <input
                    placeholder="Filter"
                    className={styles.input}
                    onChange={(e): void => handleFilter(e.target.value)}
                />
            </div>
        </div>);
};

export default LogsView;
