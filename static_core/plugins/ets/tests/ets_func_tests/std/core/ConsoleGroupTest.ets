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

class RequestHeaders {
    Authorization: string;

    constructor(Authorization: string) {
        this.Authorization = Authorization;
    }
}

class User {
    id: number;
    name: string;

    constructor(id: number, name: string) {
        this.id = id;
        this.name = name;
    }
}

class ConsoleLogger {
    static logUserDetails(): void {
        console.group("User Details");
        console.log("Name: John");
        console.log("Age: 30");
        console.log("Email: john@example.com");
        console.groupEnd();
    }

    static logApplicationInfo(): void {
        console.group("Application Info");
        console.log("Version: 1.0.0");
        
        console.group("System Details");
        console.log("OS: Windows");
        console.log("Browser: Chrome");
        console.groupEnd();
        
        console.group("Performance");
        console.log("Memory: 4GB");
        console.log("CPU: 2.4GHz");
        console.groupEnd();
        console.groupEnd();
    }

    static logDebugInfo(): void {
        console.groupCollapsed("Debug Information");
        console.log("Debug Level: Verbose");
        console.log("Stack Trace: Enabled");
        console.log("Logging: Enabled");
        console.groupEnd();
    }

    static logApiCalls(): void {
        console.group("API Calls");
        console.log("GET /users");
        
        console.groupCollapsed("Request Details");
        const headers = new RequestHeaders("Bearer token");
        console.log("Headers:", headers);
        console.groupEnd();
        
        console.groupCollapsed("Response");
        console.log("Status: 200");
        console.log("Data:", [new User(1, "John")]);
        console.groupEnd();
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
}
