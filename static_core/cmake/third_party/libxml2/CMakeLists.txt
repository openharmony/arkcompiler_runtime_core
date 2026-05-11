# Copyright (c) 2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

project(libxml2)

# Set paths
set(LIBXML2_SOURCE_DIR ${PANDA_THIRD_PARTY_SOURCES_DIR}/libxml2)
set(LIBXML2_GEN_DIR ${PROJECT_BINARY_DIR}/libxml2-2.14.0)
set(LIBXML2_INCLUDE_DIR ${PROJECT_BINARY_DIR}/include)

# List of patch files
set(LIBXML2_PATCHES
    ${LIBXML2_SOURCE_DIR}/Backport-CVE-2025-32414-python-Read-at-most-len-4-ch-c.patch
    ${LIBXML2_SOURCE_DIR}/Backport-CVE-2025-32415-schemas-Fix-heap-buffer-over-c.patch
    ${LIBXML2_SOURCE_DIR}/Fix_XML_PARSE_NOBLANKS_dropping_non-whitespace_text.patch
    ${LIBXML2_SOURCE_DIR}/Backport-CVE-2025-6021-tree-Fix-integer-overflow-in-xmlBuildQName-c.patch
    ${LIBXML2_SOURCE_DIR}/Fix-relaxng-is-parsed-to-an-infinite-attrs-next-loop.patch
    ${LIBXML2_SOURCE_DIR}/Backport-CVE-2025-6170-Fix-potential-buffer-overflow-of-interactive-shell.patch
    ${LIBXML2_SOURCE_DIR}/Fix-CVE-2025-49794-CVE-2025-49796-memory-safety-issues-in-xmlSchematronReportOutput.patch
    ${LIBXML2_SOURCE_DIR}/Fix-CVE-2025-49795-null-pointer-dereference-leading-to-DoS.patch
    ${LIBXML2_SOURCE_DIR}/Fix-CVE-2025-8732-Prevent-infinite-recursion-in-xmlCatalogList.patch
    ${LIBXML2_SOURCE_DIR}/Fix-CVE-2026-0990-catalog-prevent-inf-recursion-in-xmlCatalogXMLResolveURI.patch
    ${LIBXML2_SOURCE_DIR}/Fix-CVE-2026-0992-catalog-Ignore-repeated-nextCatalog-entries.patch
    ${LIBXML2_SOURCE_DIR}/Fix-CVE-2026-0989-Add-RelaxNG-include-limit.patch
    ${LIBXML2_SOURCE_DIR}/Fix-CVE-2026-1757-shell-free-cmdline-before-continue.patch
)

# List of generated source files (outputs from install.py)
set(LIBXML2_GENERATED_SOURCES
    ${LIBXML2_GEN_DIR}/HTMLparser.c
    ${LIBXML2_GEN_DIR}/HTMLtree.c
    ${LIBXML2_GEN_DIR}/SAX2.c
    ${LIBXML2_GEN_DIR}/buf.c
    ${LIBXML2_GEN_DIR}/c14n.c
    ${LIBXML2_GEN_DIR}/catalog.c
    ${LIBXML2_GEN_DIR}/chvalid.c
    ${LIBXML2_GEN_DIR}/debugXML.c
    ${LIBXML2_GEN_DIR}/dict.c
    ${LIBXML2_GEN_DIR}/encoding.c
    ${LIBXML2_GEN_DIR}/entities.c
    ${LIBXML2_GEN_DIR}/error.c
    ${LIBXML2_GEN_DIR}/globals.c
    ${LIBXML2_GEN_DIR}/hash.c
    ${LIBXML2_GEN_DIR}/list.c
    ${LIBXML2_GEN_DIR}/nanohttp.c
    ${LIBXML2_GEN_DIR}/parser.c
    ${LIBXML2_GEN_DIR}/parserInternals.c
    ${LIBXML2_GEN_DIR}/pattern.c
    ${LIBXML2_GEN_DIR}/relaxng.c
    ${LIBXML2_GEN_DIR}/schematron.c
    ${LIBXML2_GEN_DIR}/threads.c
    ${LIBXML2_GEN_DIR}/tree.c
    ${LIBXML2_GEN_DIR}/uri.c
    ${LIBXML2_GEN_DIR}/valid.c
    ${LIBXML2_GEN_DIR}/xinclude.c
    ${LIBXML2_GEN_DIR}/xlink.c
    ${LIBXML2_GEN_DIR}/xmlIO.c
    ${LIBXML2_GEN_DIR}/xmlmemory.c
    ${LIBXML2_GEN_DIR}/xmlmodule.c
    ${LIBXML2_GEN_DIR}/xmlreader.c
    ${LIBXML2_GEN_DIR}/xmlregexp.c
    ${LIBXML2_GEN_DIR}/xmlsave.c
    ${LIBXML2_GEN_DIR}/xmlschemas.c
    ${LIBXML2_GEN_DIR}/xmlschemastypes.c
    ${LIBXML2_GEN_DIR}/xmlstring.c
    ${LIBXML2_GEN_DIR}/xmlunicode.c
    ${LIBXML2_GEN_DIR}/xmlwriter.c
    ${LIBXML2_GEN_DIR}/xpath.c
    ${LIBXML2_GEN_DIR}/xpointer.c
    ${LIBXML2_GEN_DIR}/xzlib.c
    ${LIBXML2_GEN_DIR}/shell.c
    ${LIBXML2_GEN_DIR}/xmllint.c
    ${LIBXML2_GEN_DIR}/config.h.cmake.in
    ${LIBXML2_GEN_DIR}/include/libxml/xmlversion.h.in
)

# ============================================================================
# libxml2_install_action - Execute install.py to extract and apply patches
# ============================================================================
add_custom_command(
    OUTPUT
        ${LIBXML2_GENERATED_SOURCES}

    COMMAND
        python3 ${LIBXML2_SOURCE_DIR}/install.py
        --gen-dir ${PROJECT_BINARY_DIR}
        --source-file ${LIBXML2_SOURCE_DIR}

    DEPENDS
        ${LIBXML2_SOURCE_DIR}/libxml2-2.14.0.tar.xz
        ${LIBXML2_PATCHES}

    COMMENT "[libxml2] Extracting tarball and applying patches..."
    VERBATIM
)

add_custom_target(libxml2_install_action
    DEPENDS
        ${LIBXML2_GENERATED_SOURCES}
)

# ============================================================================
# libxml2_generate_header - Execute generate_header.py to generate config headers
# ============================================================================
set(LIBXML2_CONFIG_INPUT ${LIBXML2_GEN_DIR}/config.h.cmake.in)
set(LIBXML2_XMLVERSION_INPUT ${LIBXML2_GEN_DIR}/include/libxml/xmlversion.h.in)
set(LIBXML2_CONFIG_OUTPUT ${LIBXML2_INCLUDE_DIR}/config.h)
set(LIBXML2_XMLVERSION_OUTPUT ${LIBXML2_INCLUDE_DIR}/libxml/xmlversion.h)

if(MSVC OR MINGW)
    set(LIBXML2_CONFIG_JSON ${LIBXML2_SOURCE_DIR}/config_win.json)
else()
    set(LIBXML2_CONFIG_JSON ${LIBXML2_SOURCE_DIR}/config_linux.json)
endif()

set(LIBXML2_XMLVERSION_JSON ${LIBXML2_SOURCE_DIR}/xml_version.json)

add_custom_command(
    OUTPUT
        ${LIBXML2_CONFIG_OUTPUT}
        ${LIBXML2_XMLVERSION_OUTPUT}

    COMMAND
        python3 ${LIBXML2_SOURCE_DIR}/generate_header.py
        --config-input-path ${LIBXML2_CONFIG_INPUT}
        --config-path ${LIBXML2_CONFIG_OUTPUT}
        --xmlversion-input-path ${LIBXML2_XMLVERSION_INPUT}
        --xmlversion-path ${LIBXML2_XMLVERSION_OUTPUT}
        --config-json ${LIBXML2_CONFIG_JSON}
        --xmlversion-json ${LIBXML2_XMLVERSION_JSON}

    DEPENDS
        ${LIBXML2_CONFIG_INPUT}
        ${LIBXML2_XMLVERSION_INPUT}
        ${LIBXML2_CONFIG_JSON}
        ${LIBXML2_XMLVERSION_JSON}

    COMMENT "[libxml2] Generating configuration headers..."
    VERBATIM
)

add_custom_target(libxml2_generate_header
    DEPENDS
        ${LIBXML2_CONFIG_OUTPUT}
        ${LIBXML2_XMLVERSION_OUTPUT}
)

# ============================================================================
# libxml2 shared library
# ============================================================================

# Filter out .in files from sources
list(FILTER LIBXML2_GENERATED_SOURCES EXCLUDE REGEX ".*\.in$")

panda_add_library(libxml2 SHARED
    ${LIBXML2_GENERATED_SOURCES}
)

set_target_properties(libxml2 PROPERTIES
    OUTPUT_NAME xml2
)

# Add dependencies to ensure scripts run before compilation
add_dependencies(libxml2 libxml2_install_action libxml2_generate_header)

# Include directories
panda_target_include_directories(libxml2 SYSTEM PUBLIC
    ${LIBXML2_INCLUDE_DIR}
    ${LIBXML2_GEN_DIR}/include
)

# Compile definitions
panda_target_compile_definitions(libxml2 PRIVATE
    HAVE_CONFIG_H
    _REENTRANT
)

if(UNIX AND NOT APPLE)
    panda_target_compile_definitions(libxml2 PRIVATE
        _GNU_SOURCE
    )
endif()

# Compile options (suppress warnings)
panda_target_compile_options(libxml2 PRIVATE
    -Wno-empty-body
    -Wno-incompatible-pointer-types
    -Wno-missing-field-initializers
    -Wno-self-assign
    -Wno-sign-compare
    -Wno-tautological-pointer-compare
    -Wno-unused-function
    -Wno-enum-compare
    -Wno-int-conversion
    -Wno-uninitialized
    -Wno-implicit-fallthrough
)

# Link libraries
if(UNIX AND NOT APPLE)
    panda_target_link_libraries(libxml2 PRIVATE dl)
elseif(MSVC OR MINGW)
    panda_target_link_libraries(libxml2 PRIVATE bcrypt ws2_32)
endif()

# iOS specific configuration
if(APPLE AND IOS)
    set_target_properties(libxml2 PROPERTIES
        OUTPUT_NAME xml2_shared
        INSTALL_NAME_DIR @rpath/libxml2_shared.framework
    )
    set_target_properties(libxml2 PROPERTIES
        MACOSX_FRAMEWORK_IDENTIFIER com.example.libxml2
    )
endif()