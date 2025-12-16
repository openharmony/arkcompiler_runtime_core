/**
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

#include "mem_manager.h"

#include "libarkbase/mem/arena_allocator.h"
#include "libarkbase/mem/pool_manager.h"

namespace libabckit {

size_t MemManager::compilerMemSize_ = {};
bool MemManager::isInitialized_ = false;

void MemManager::InitializeMemory()
{
    ark::mem::MemConfig::Initialize(0, 0, MemManager::compilerMemSize_, 0, 0, 0);
    ark::PoolManager::Initialize(ark::PoolType::MMAP);
    MemManager::isInitialized_ = true;
}

void MemManager::Finalize()
{
    if (MemManager::isInitialized_) {
        ark::PoolManager::Finalize();
        ark::mem::MemConfig::Finalize();
        MemManager::isInitialized_ = false;
    }
}

size_t MemManager::GetCompilerSpaceCurrentSize()
{
    ASSERT(isInitialized_);
    return ark::PoolManager::GetMmapMemPool()->GetCompilerSpaceCurrentSize();
}

size_t MemManager::GetCompilerSpaceMaxSize()
{
    ASSERT(isInitialized_);
    return ark::PoolManager::GetMmapMemPool()->GetCompilerSpaceMaxSize();
}

bool MemManager::IsBelowThreshold(size_t maxMemSize, size_t maxMemPercent)
{
    if (maxMemSize > 0) {
        size_t compilerSpaceCurrentSize = MemManager::GetCompilerSpaceCurrentSize();
        if (compilerSpaceCurrentSize > maxMemSize) {
            return false;
        }
    }
    if (maxMemPercent > 0) {
        size_t compilerSpaceCurrentSize = MemManager::GetCompilerSpaceCurrentSize();
        size_t compilerSpaceMaxSize = MemManager::GetCompilerSpaceMaxSize();
        size_t percent = (compilerSpaceMaxSize > 0 && compilerSpaceCurrentSize > 0)
                             ? ((compilerSpaceCurrentSize * 100) / compilerSpaceMaxSize)
                             : 0;
        if (percent > maxMemPercent) {
            return false;
        }
    }
    return true;
}

}  // namespace libabckit
