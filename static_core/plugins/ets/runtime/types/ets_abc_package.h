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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ABC_PACKAGE_H_
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ABC_PACKAGE_H_

#include <memory>
#include <string>
#include <string_view>

#include "libarkfile/file.h"

namespace ark::ets {

/**
 * @brief Reading of the abc entry from an application package (`.hap` / `.hsp` / `.hqf` shaped).
 *
 * Single authority for the package format knowledge shared by the abc loader
 * (`arkruntime_AbcFile.cpp`) and cold reload: the entry names, the derived path a package is
 * recorded under, and the entry-reading styles. Which package suffixes a caller accepts stays
 * the caller's policy: the loader handles `.hap` / `.hsp` only, cold reload also handles the
 * `.hqf` patch format.
 */

/** Suffix of a shared package (`.hsp`). */
inline constexpr std::string_view HSP_SUFFIX = ".hsp";
/** Suffix of an application package (`.hap`). */
inline constexpr std::string_view HAP_SUFFIX = ".hap";
/** Suffix of a quick-fix patch package. */
inline constexpr std::string_view HQF_SUFFIX = ".hqf";
/** Path of the abc entry inside a package, appended to the package path without suffix. */
inline constexpr std::string_view PACKAGE_ABC_PATH = "/ets/modules_static.abc";
/** Name of the abc entry inside a `.hap` / `.hqf` package. */
inline constexpr std::string_view PACKAGE_ABC_ENTRY = "ets/modules_static.abc";

/**
 * @returns the path a package's abc is recorded under: the package path without suffix plus
 * PACKAGE_ABC_PATH.
 */
std::string GetAbcPathFromPackagePath(std::string_view packagePath, std::string_view suffix);

/** @returns whether @a path ends with @a suffix. */
bool EndsWith(std::string_view path, std::string_view suffix);

/** @brief Outcome of reading a package's abc entry. */
enum class PackageReadResult {
    OK,            ///< The abc entry was read and opened; `outPf` holds it.
    OPEN_FAILED,   ///< The package itself cannot be opened (the extractor rejects it).
    NO_ABC_ENTRY,  ///< The package is readable, but carries no abc entry (a native-only patch).
};

/**
 * @brief Read the abc entry of a package into `outPf`, opening it under the derived path.
 *
 * Entry-reading style follows the format rules: a `.hsp` is looked up by its derived-path entry
 * without an integrity check, any other package by the fixed entry name with the check (both
 * established by the loader; see `PACKAGE_ABC_ENTRY`).
 *
 * The mapping of the outcomes is the caller's: the loader treats both failures as load errors
 * (an exception for OPEN_FAILED, a null return for NO_ABC_ENTRY), cold reload treats
 * NO_ABC_ENTRY as "nothing to prepend".
 *
 * @param packagePath path of the package file.
 * @param suffix the package suffix of @a packagePath (HSP_SUFFIX selects the hsp entry style).
 * @param[out] outPf receives the opened file on OK.
 */
PackageReadResult ReadPackageAbc(const std::string &packagePath, std::string_view suffix,
                                 std::unique_ptr<const panda_file::File> &outPf);

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ABC_PACKAGE_H_
