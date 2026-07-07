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

function main() {
    // Basic counter
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.count();
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.count();
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.count();
    
    // Named counter
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.count('userClick');
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.count('userClick');
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.count('userClick');
    
    // Multiple named counters
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.count('loop1');
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.count('loop2');
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.count('loop1');
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.count('loop2');
    
    // Reset default counter
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.countReset();
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.count();
    
    // Reset named counter
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.count('userAction');
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.count('userAction');
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.countReset('userAction');
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.count('userAction');
    
    // Counter in loops
    for (let i = 0; i < 3; i++) {
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.count('forLoop');
    }
    
    // Reset and reuse
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.count('test');
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.countReset('test');
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.count('test');
    
    // Multiple resets
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.count('multi');
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.countReset('multi');
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.count('multi');
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.countReset('multi');
}
