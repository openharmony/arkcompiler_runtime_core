/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
    id: number;
    filter: string;

    constructor(id: number, filter: string) {
        this.id = id;
        this.filter = filter;
    }
}

class SystemInfo {
    memory: string;
    cpu: string;

    constructor(memory: string, cpu: string) {
        this.memory = memory;
        this.cpu = cpu;
    }
}

class Info {
    level: string;
    message: string;
    details: SystemInfo;

    constructor(level: string, message: string, details: SystemInfo) {
        this.level = level;
        this.message = message;
        this.details = details;
    }
}

class Connection {
    id: number;
    user: string;

    constructor(id: number, user: string) {
        this.id = id;
        this.user = user;
    }
}

function main() {
    // Basic debug messages
    console.debug("Debug message");
    console.debug(42);
    console.debug(undefined);
    
    // Multiple arguments
    console.debug("Debug:", "Database connection established");
    console.debug("Query params:", new QueryParams(1, "active"));
    
    // Debug with objects
    const systemInfo = new SystemInfo("1.2GB", "45%");
    const debugData = new Info(
        "debug",
        "System check",
        systemInfo
    );
    console.debug("System status:", debugData);
    
    // Debug arrays
    console.debug("Active connections:", [
        new Connection(1, "user1"),
        new Connection(2, "user2")
    ]);
    
    // Debug with Error objects
    try {
        throw new Error("Test error");
    } catch (error) {
        console.debug("Caught error:", error);
    }
}
