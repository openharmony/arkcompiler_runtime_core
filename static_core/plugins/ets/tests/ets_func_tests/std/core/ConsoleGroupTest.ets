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

class RequestHeaders {
    public authorization: string;

    constructor(authorization: string) {
        this.authorization = authorization;
    }
}

class User {
    public id: number;
    public name: string;

    constructor(id: number, name: string) {
        this.id = id;
        this.name = name;
    }
}

class ConsoleLogger {
    static logUserDetails(): void {
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.group('User Details');
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.log('Name: John');
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.log('Age: 30');
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.log('Email: john@example.com');
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.groupEnd();
    }

    static logApplicationInfo(): void {
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.group('Application Info');
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.log('Version: 1.0.0');

        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.group('System Details');
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.log('OS: Windows');
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.log('Browser: Chrome');
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.groupEnd();

        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.group('Performance');
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.log('Memory: 4GB');
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.log('CPU: 2.4GHz');
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.groupEnd();
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.groupEnd();
    }

    static logDebugInfo(): void {
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.groupCollapsed('Debug Information');
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.log('Debug Level: Verbose');
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.log('Stack Trace: Enabled');
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.log('Logging: Enabled');
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.groupEnd();
    }

    static logApiCalls(): void {
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.group('API Calls');
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.log('GET /users');

        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.groupCollapsed('Request Details');
        const headers = new RequestHeaders('Bearer token');
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.log('Headers:', headers);
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.groupEnd();

        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.groupCollapsed('Response');
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.log('Status: 200');
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.log('Data:', [new User(1, 'John')] as FixedArray<User>);
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.groupEnd();
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.groupEnd();
    }
}

function main() {
    // Basic group
    ConsoleLogger.logUserDetails();

    // Nested groups
    ConsoleLogger.logApplicationInfo();

    // Collapsed group
    ConsoleLogger.logDebugInfo();

    // Mix of normal and collapsed groups
    ConsoleLogger.logApiCalls();

    // Group
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log('outter');
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.group();
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log('level 1');
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.group('in level 1');
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log('level 2');

    // Reset for next test
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.groupEnd()
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.groupEnd()

    // Collapsed
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log('outter Collapsed');
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.groupCollapsed();
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log('level 1 Collapsed');
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.groupCollapsed('in level 1 Collapsed');
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log('level 2 Collapsed');

    // Reset for next test
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.groupEnd()
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.groupEnd()

    let num = 1
    let str = 'str'
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.groupCollapsed(num, str)
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.groupCollapsed(str)
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.groupCollapsed('count %d')
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.groupCollapsed()
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.groupCollapsed(num, str, 'sdsf')
}
