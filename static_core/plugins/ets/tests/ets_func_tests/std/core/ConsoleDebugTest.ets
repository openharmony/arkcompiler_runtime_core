/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
class QueryParams {
    public id: number;
    public filter: string;

    constructor(id: number, filter: string) {
        this.id = id;
        this.filter = filter;
    }
}

class SystemInfo {
    public memory: string;
    public cpu: string;

    constructor(memory: string, cpu: string) {
        this.memory = memory;
        this.cpu = cpu;
    }
}

class Info {
    public level: string;
    public message: string;
    public details: SystemInfo;

    constructor(level: string, message: string, details: SystemInfo) {
        this.level = level;
        this.message = message;
        this.details = details;
    }
}

function main() {
    // Basic debug messages
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.debug('Debug message');
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.debug(42);
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.debug(undefined);
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.debug(null);
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.debug()

    // Multiple arguments
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.debug('Debug:', 'Database connection established');
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.debug('Query params:', new QueryParams(1, 'active'));

    // Debug with objects
    const systemInfo = new SystemInfo('1.2GB', '45%');
    const debugData = new Info(
        'debug',
        'System check',
        systemInfo
    );
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.debug('System status:', debugData);

    // Debug with Error objects
    try {
        throw new Error('Test error');
    } catch (error) {
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.debug('Caught error:', error);
    }

    // Formatted strings
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.debug('%d')
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.debug('%d', 1)
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.debug('%d', 1, 2)

    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.debug('%s %s', '37', 37)
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.debug('%s %s', '19.5', 19.5)
}
