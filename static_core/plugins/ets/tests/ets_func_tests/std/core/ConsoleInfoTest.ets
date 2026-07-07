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
class Config {
    public port: number;
    public environment: string;
    public database: string;
    public cache: string;

    constructor(port: number, environment: string, database: string, cache: string) {
        this.port = port;
        this.environment = environment;
        this.database = database;
        this.cache = cache;
    }

    toString(): string {
        return `Config(port=${this.port}, env=${this.environment}, db=${this.database}, cache=${this.cache})`;
    }
}

class ApplicationLogger {
    private static features: FixedArray<string> = [
        'authentication',
        'authorization',
        'caching',
        'logging'
    ];

    static logStartup(): void {
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.info('Application started');
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.info('Version 1.0.0');
    }

    static logServerStatus(port: number): void {
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.info('Server status:', 'Running', 'Port:', port);
    }

    static logConfiguration(config: Config): void {
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.info('Server configuration:', config);
    }

    static logActiveFeatures(): void {
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.info('Active features:', ApplicationLogger.features);
    }

    static logSystemInformation(): void {
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.info(
            '%c System Information',
            'color: green;'
        );
    }

}

function main() {
    // Basic info messages
    ApplicationLogger.logStartup();

    // Info with multiple arguments
    ApplicationLogger.logServerStatus(3000);

    // Info with objects
    const serverConfig = new Config(
        3000,
        'production',
        'mongodb',
        'redis'
    );
    ApplicationLogger.logConfiguration(serverConfig);

    // Info with arrays
    ApplicationLogger.logActiveFeatures();

    // Info with styled output
    ApplicationLogger.logSystemInformation();

    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.info()

    // Formatted strings
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.info('%d')
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.info('%d', 1)
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.info('%d', 1, 2)

    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.info('%s %s', '37', 37)
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.info('%s %s', '19.5', 19.5)
}
