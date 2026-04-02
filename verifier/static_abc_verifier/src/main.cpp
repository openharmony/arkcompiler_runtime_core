/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "verifier.h"

#include <iostream>
#include <string>

static void PrintUsage(const char *progName)
{
    std::cerr << "Usage: " << progName << " <input.abc>" << std::endl;
    std::cerr << "Verify a static ABC file for structural integrity." << std::endl;
}

int main(int argc, const char **argv)
{
    constexpr int kCliMinArgCount = 2;
    if (argc < kCliMinArgCount) {
        PrintUsage(argv[0]);
        return 1;
    }

    std::string inputFile = argv[1];

    if (inputFile == "--help" || inputFile == "-h") {
        PrintUsage(argv[0]);
        return 0;
    }

    ark::static_verifier::StaticAbcVerifier verifier(inputFile);
    auto result = verifier.Verify();
    if (result.valid) {
        std::cout << "PASS: " << inputFile << " is a valid static ABC file." << std::endl;
        return 0;
    }

    std::cerr << "FAIL: " << inputFile << " verification failed:" << std::endl;
    for (const auto &err : result.errors) {
        std::cerr << "  [E" << static_cast<int>(err.code) << "] " << err.message << std::endl;
    }
    return 1;
}
