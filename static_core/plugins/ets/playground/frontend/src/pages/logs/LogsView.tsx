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

import React, {useEffect, useState} from 'react';
import {ILog} from '../../models/logs';
import styles from './styles.module.scss';
import {Icon} from '@blueprintjs/core';
import cx from 'classnames';


interface IProps {
    logArr: ILog[]
    clearFilters: () => void
}

const LogsView = ({logArr, clearFilters}: IProps): JSX.Element => {
    const [logs, setLogs] = useState<ILog[]>(logArr);

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

    return (
        <div className={styles.container}>
            <div className={styles.containerLogs}>
                {logs.map((log: ILog, index) => (
                    <span key={index} className={styles.rowContainer}>
                        <span
                            className={cx({
                                [styles.tag]: true,
                                [styles.red]: log?.from?.includes('Err')})}
                        >
                            [{log?.from?.includes('Err') ? 'ERR' : 'LOG'}]:
                        </span>
                        <span className={styles.logText}>{log.message}</span>
                    </span>
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
