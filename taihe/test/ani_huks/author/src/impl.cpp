/*
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
#include <cstdint>
#include <iostream>

#include "huks.impl.hpp"
#include "stdexcept"
#include "taihe/array.hpp"

using namespace taihe;

namespace {
std::string dataAsString(array_view<uint8_t> data)
{
    std::string res;
    int tempnum = 16;
    for (int i = 0; i < data.size(); i++) {
        uint8_t c = static_cast<uint8_t>(data[i]);
        static constexpr char const *const TEST_STR = "0123456789abcdef";
        res += TEST_STR[data[i] / tempnum];
        res += TEST_STR[data[i] % tempnum];
    }
    return res;
}

::huks::HuksResult generateKey(string_view keyAlias, ::huks::HuksOptions const &options)
{
    std::cout << "keyAlias: " << keyAlias << std::endl;
    if (auto inData = options.inData) {
        std::cout << "inData = " << dataAsString(*inData) << std::endl;
    } else {
        std::cout << "No inData!" << std::endl;
    }
    if (auto properties = options.properties) {
        std::cout << "Properties:" << std::endl;
        for (auto const &property : *properties) {
            std::cout << "tag = " << (size_t)property.tag.get_value() << std::endl;
            switch (property.value.get_tag()) {
                case huks::HuksParamValue::tag_t::bigintValue:
                    std::cout << "bigint " << property.value.get_bigintValue_ref() << std::endl;
                    break;
                case huks::HuksParamValue::tag_t::booleanValue:
                    std::cout << "boolean " << property.value.get_booleanValue_ref() << std::endl;
                    break;
                case huks::HuksParamValue::tag_t::bufferValue:
                    std::cout << "array " << dataAsString(property.value.get_bufferValue_ref()) << std::endl;
                    break;
                case huks::HuksParamValue::tag_t::numberValue:
                    std::cout << "number " << property.value.get_numberValue_ref() << std::endl;
                    break;
            }
        }
    } else {
        std::cout << "No Properties!" << std::endl;
    }
    huks::HuksParam huksParam = {
        .tag = huks::HuksTag::key_t::HUKS_TAG_ACCESS_TIME,
        .value = huks::HuksParamValue::make_bufferValue(array<uint8_t>::make(4, 0xcc)),
    };
    huks::HuksResult huksResult = {
        .errorCode = 0.0,
        .outData = optional<array<uint8_t>>::make(array<uint8_t>::make(3, 0x12)),
        .properties = optional<array<huks::HuksParam>>::make(array<huks::HuksParam>::make(7, huksParam)),
        .certChains = optional<array<string>>::make(array<string>::make(5, "Hello")),
    };
    return huksResult;
}

void generateKeyItem(string_view keyAlias, ::huks::HuksOptions const &options)
{
    throw std::runtime_error("Function generateKeyItem Not implemented");
}

::huks::HuksResult deleteKey(string_view keyAlias, ::huks::HuksOptions const &options)
{
    throw std::runtime_error("Function deleteKey Not implemented");
}

void deleteKeyItem(string_view keyAlias, ::huks::HuksOptions const &options)
{
    throw std::runtime_error("Function deleteKeyItem Not implemented");
}

::huks::HuksResult importKey(string_view keyAlias, ::huks::HuksOptions const &options)
{
    throw std::runtime_error("Function importKey Not implemented");
}

void importKeyItem(string_view keyAlias, ::huks::HuksOptions const &options)
{
    throw std::runtime_error("Function importKeyItem Not implemented");
}

void importWrappedKeyItem(string_view keyAlias, string_view wrappingKeyAlias, ::huks::HuksOptions const &options)
{
    throw std::runtime_error("Function importWrappedKeyItem Not implemented");
}

::huks::HuksResult exportKey(string_view keyAlias, ::huks::HuksOptions const &options)
{
    throw std::runtime_error("Function exportKey Not implemented");
}

::huks::HuksReturnResult exportKeyItem(string_view keyAlias, ::huks::HuksOptions const &options)
{
    throw std::runtime_error("Function exportKeyItem Not implemented");
}

::huks::HuksResult getKeyProperties(string_view keyAlias, ::huks::HuksOptions const &options)
{
    throw std::runtime_error("Function getKeyProperties Not implemented");
}

::huks::HuksReturnResult getKeyItemProperties(string_view keyAlias, ::huks::HuksOptions const &options)
{
    throw std::runtime_error("Function getKeyItemProperties Not implemented");
}

bool isKeyExist(string_view keyAlias, ::huks::HuksOptions const &options)
{
    throw std::runtime_error("Function isKeyExist Not implemented");
}

bool isKeyItemExist(string_view keyAlias, ::huks::HuksOptions const &options)
{
    throw std::runtime_error("Function isKeyItemExist Not implemented");
}

bool hasKeyItem(string_view keyAlias, ::huks::HuksOptions const &options)
{
    throw std::runtime_error("Function hasKeyItem Not implemented");
}

::huks::HuksHandle init(string_view keyAlias, ::huks::HuksOptions const &options)
{
    throw std::runtime_error("Function init Not implemented");
}

::huks::HuksSessionHandle initSession(string_view keyAlias, ::huks::HuksOptions const &options)
{
    throw std::runtime_error("Function initSession Not implemented");
}

::huks::HuksResult update(double handle, optional_view<array<uint8_t>> token, ::huks::HuksOptions const &options)
{
    throw std::runtime_error("Function update Not implemented");
}

::huks::HuksReturnResult updateSession(double handle, ::huks::HuksOptions const &options,
                                       optional_view<array<uint8_t>> token)
{
    throw std::runtime_error("Function updateSession Not implemented");
}

::huks::HuksResult finish(double handle, ::huks::HuksOptions const &options)
{
    throw std::runtime_error("Function finish Not implemented");
}

::huks::HuksReturnResult finishSession(double handle, ::huks::HuksOptions const &options,
                                       optional_view<array<uint8_t>> token)
{
    throw std::runtime_error("Function finishSession Not implemented");
}

::huks::HuksResult abort(double handle, ::huks::HuksOptions const &options)
{
    throw std::runtime_error("Function abort Not implemented");
}

void abortSession(double handle, ::huks::HuksOptions const &options)
{
    throw std::runtime_error("Function abortSession Not implemented");
}

::huks::HuksReturnResult attestKeyItem(string_view keyAlias, ::huks::HuksOptions const &options)
{
    throw std::runtime_error("Function attestKeyItem Not implemented");
}

::huks::HuksReturnResult anonAttestKeyItem(string_view keyAlias, ::huks::HuksOptions const &options)
{
    throw std::runtime_error("Function anonAttestKeyItem Not implemented");
}

string getSdkVersion(::huks::HuksOptions const &options)
{
    throw std::runtime_error("Function getSdkVersion Not implemented");
}

::huks::HuksListAliasesReturnResult listAliases(::huks::HuksOptions const &options)
{
    throw std::runtime_error("Function listAliases Not implemented");
}
}  // namespace
// The macros used below are automatically generated code.
// NOLINTBEGIN
TH_EXPORT_CPP_API_generateKey(generateKey);
TH_EXPORT_CPP_API_generateKeyItem(generateKeyItem);
TH_EXPORT_CPP_API_deleteKey(deleteKey);
TH_EXPORT_CPP_API_deleteKeyItem(deleteKeyItem);
TH_EXPORT_CPP_API_importKey(importKey);
TH_EXPORT_CPP_API_importKeyItem(importKeyItem);
TH_EXPORT_CPP_API_importWrappedKeyItem(importWrappedKeyItem);
TH_EXPORT_CPP_API_exportKey(exportKey);
TH_EXPORT_CPP_API_exportKeyItem(exportKeyItem);
TH_EXPORT_CPP_API_getKeyProperties(getKeyProperties);
TH_EXPORT_CPP_API_getKeyItemProperties(getKeyItemProperties);
TH_EXPORT_CPP_API_isKeyExist(isKeyExist);
TH_EXPORT_CPP_API_isKeyItemExist(isKeyItemExist);
TH_EXPORT_CPP_API_hasKeyItem(hasKeyItem);
TH_EXPORT_CPP_API_init(init);
TH_EXPORT_CPP_API_initSession(initSession);
TH_EXPORT_CPP_API_update(update);
TH_EXPORT_CPP_API_updateSession(updateSession);
TH_EXPORT_CPP_API_finish(finish);
TH_EXPORT_CPP_API_finishSession(finishSession);
TH_EXPORT_CPP_API_abort(abort);
TH_EXPORT_CPP_API_abortSession(abortSession);
TH_EXPORT_CPP_API_attestKeyItem(attestKeyItem);
TH_EXPORT_CPP_API_anonAttestKeyItem(anonAttestKeyItem);
TH_EXPORT_CPP_API_getSdkVersion(getSdkVersion);
TH_EXPORT_CPP_API_listAliases(listAliases);
// NOLINTEND
