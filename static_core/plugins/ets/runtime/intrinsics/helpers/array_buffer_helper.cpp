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

#include "array_buffer_helper.h"

#include <array>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <cctype>

#include "plugins/ets/runtime/types/ets_arraybuffer.h"

namespace ark::ets::intrinsics::helpers {

namespace base64 {

constexpr std::string_view K_BASE64_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Lookup table size for all possible byte values.
constexpr size_t K_LOOKUP_TABLE_SIZE = 256;

// Base64 block and binary block size constants.
constexpr size_t K_BASE64_BLOCK_SIZE = 4;  // Size of a Base64 encoded block.
constexpr size_t K_MAX_PADDING_CHARS = 2;  // Maximum number of padding characters allowed.
constexpr size_t K_BINARY_BLOCK_SIZE = 3;  // Size of binary data block that becomes one Base64 block.

// Bit manipulation constants.
constexpr size_t K_BITS_PER_BYTE = 8;
constexpr uint32_t K_BASE64_MASK = 0x3F;  // Mask for 6 bits (Base64 character).

// Bit shift constants for Base64 encoding/decoding.
constexpr size_t K_FIRST_CHAR_SHIFT = 2;
constexpr size_t K_SECOND_CHAR_SHIFT = 4;
constexpr size_t K_THIRD_CHAR_SHIFT = 6;
constexpr size_t K_HIGH_BYTE_SHIFT = 16;
constexpr size_t K_MID_BYTE_SHIFT = 12;
constexpr size_t K_LOW_BYTE_SHIFT = 6;
constexpr size_t K_LAST_CHAR_SHIFT = 18;

constexpr unsigned int K_TWO_BIT_MASK = 0x3;

constexpr char K_PADDING_CHAR = '=';

constexpr std::array<int, K_LOOKUP_TABLE_SIZE> BuildDecodingTable() noexcept
{
    std::array<int, K_LOOKUP_TABLE_SIZE> table {};
    for (auto &entry : table) {
        entry = -1;  // mark as invalid
    }
    for (size_t i = 0; i < K_BASE64_CHARS.size(); ++i) {
        table[static_cast<unsigned char>(K_BASE64_CHARS[i])] = static_cast<int>(i);
    }
    return table;
}

constexpr auto K_DECODING_TABLE = BuildDecodingTable();

[[nodiscard]] constexpr bool IsBase64Character(unsigned char c) noexcept
{
    return K_DECODING_TABLE[c] != -1;
}

[[nodiscard]] bool ValidateBase64Input(std::string_view input) noexcept
{
    if (input.empty()) {
        return true;
    }
    if (input.size() % K_BASE64_BLOCK_SIZE != 0) {
        return false;
    }

    const auto paddingStart = std::find(input.begin(), input.end(), K_PADDING_CHAR);
    const bool hasPadding = paddingStart != input.end();
    // Count padding characters at the end
    const auto paddingCount = static_cast<size_t>(std::count(paddingStart, input.end(), K_PADDING_CHAR));
    const bool validChars =
        std::all_of(input.begin(), paddingStart, [](unsigned char c) { return IsBase64Character(c); });
    const bool validPadding =
        !hasPadding || (paddingCount <= K_MAX_PADDING_CHARS &&
                        std::all_of(paddingStart, input.end(), [](char c) { return c == K_PADDING_CHAR; }));
    return validChars && validPadding;
}

[[nodiscard]] PandaString Decode(std::string_view encodedData)
{
    if (encodedData.empty()) {
        return {};
    }
    const size_t kPaddingOffset = 2;
    size_t pad = 0;
    if (!encodedData.empty() && encodedData.back() == K_PADDING_CHAR) {
        ++pad;
    }
    if (encodedData.size() >= kPaddingOffset && encodedData[encodedData.size() - kPaddingOffset] == K_PADDING_CHAR) {
        ++pad;
    }

    PandaString decoded;
    decoded.reserve((encodedData.size() / K_BASE64_BLOCK_SIZE) * K_BINARY_BLOCK_SIZE - pad);

    const int kSecondCharOffset = 2;
    const int kThirdCharOffset = 3;

    for (size_t i = 0; i < encodedData.size(); i += K_BASE64_BLOCK_SIZE) {
        int decodedChar0 = K_DECODING_TABLE[static_cast<unsigned char>(encodedData[i])];
        int decodedChar1 = K_DECODING_TABLE[static_cast<unsigned char>(encodedData[i + 1])];
        int decodedChar2 = (encodedData[i + kSecondCharOffset] == K_PADDING_CHAR)
                               ? 0
                               : K_DECODING_TABLE[static_cast<unsigned char>(encodedData[i + kSecondCharOffset])];
        int decodedChar3 = (encodedData[i + kThirdCharOffset] == K_PADDING_CHAR)
                               ? 0
                               : K_DECODING_TABLE[static_cast<unsigned char>(encodedData[i + kThirdCharOffset])];

        auto unsignedChar0 = static_cast<unsigned int>(decodedChar0);
        auto unsignedChar1 = static_cast<unsigned int>(decodedChar1);
        auto unsignedChar2 = static_cast<unsigned int>(decodedChar2);
        auto unsignedChar3 = static_cast<unsigned int>(decodedChar3);

        decoded.push_back(
            static_cast<char>((unsignedChar0 << K_FIRST_CHAR_SHIFT) | (unsignedChar1 >> K_SECOND_CHAR_SHIFT)));
        if (encodedData[i + kSecondCharOffset] != K_PADDING_CHAR) {
            decoded.push_back(static_cast<char>(((unsignedChar1 & K_BASE64_MASK) << K_SECOND_CHAR_SHIFT) |
                                                (unsignedChar2 >> K_FIRST_CHAR_SHIFT)));
        }
        if (encodedData[i + kThirdCharOffset] != K_PADDING_CHAR) {
            decoded.push_back(
                static_cast<char>(((unsignedChar2 & K_TWO_BIT_MASK) << K_THIRD_CHAR_SHIFT) | unsignedChar3));
        }
    }
    return decoded;
}

[[nodiscard]] PandaString Encode(const PandaVector<uint8_t> &binaryData)
{
    if (binaryData.empty()) {
        return {};
    }
    PandaString encoded;
    const size_t kReserveMultiplier = 2;

    encoded.reserve(((binaryData.size() + kReserveMultiplier) / K_BINARY_BLOCK_SIZE) * K_BASE64_BLOCK_SIZE);

    const size_t kOneByte = 1;
    const size_t kTwoBytes = 2;

    size_t pos = 0;
    while (pos < binaryData.size()) {
        size_t remain = binaryData.size() - pos;
        uint32_t triple = (static_cast<uint32_t>(binaryData[pos]) << K_HIGH_BYTE_SHIFT);
        if (remain > kOneByte) {
            triple |= (static_cast<uint32_t>(binaryData[pos + kOneByte]) << K_BITS_PER_BYTE);
        }
        if (remain > kTwoBytes) {
            triple |= static_cast<uint32_t>(binaryData[pos + kTwoBytes]);
        }

        encoded.push_back(K_BASE64_CHARS[(triple >> K_LAST_CHAR_SHIFT) & K_BASE64_MASK]);
        encoded.push_back(K_BASE64_CHARS[(triple >> K_MID_BYTE_SHIFT) & K_BASE64_MASK]);
        encoded.push_back(remain > kOneByte ? K_BASE64_CHARS[(triple >> K_LOW_BYTE_SHIFT) & K_BASE64_MASK]
                                            : K_PADDING_CHAR);
        encoded.push_back(remain > kTwoBytes ? K_BASE64_CHARS[triple & K_BASE64_MASK] : K_PADDING_CHAR);

        pos += K_BINARY_BLOCK_SIZE;
    }
    return encoded;
}

}  // namespace base64

namespace encoding {

using namespace std::literals::string_view_literals;
// UTF-16 related constants.
constexpr size_t K_UTF16_BYTES_PER_CHAR = 2;  // Number of bytes per UTF-16 character.
constexpr size_t K_HIGH_BYTE_SHIFT = 8;       // Shift for high byte in UTF-16.

constexpr size_t HEX_BASE = 16;
constexpr size_t K_HEX_PAIR_SIZE = 2;

// Named constants for bit masks
constexpr uint8_t K_ASCII_MASK = 0x7F;
constexpr uint8_t K_BYTE_MASK = 0xFF;

constexpr std::array K_SINGLE_BYTE_ENCODINGS = {"utf8"sv, "utf-8"sv, "ascii"sv, "latin1"sv, "binary"sv};
constexpr std::array K_DOUBLE_BYTE_ENCODINGS = {"utf16le"sv, "ucs2"sv, "ucs-2"sv};

constexpr std::array UTF8_ENCODINGS = {"utf8"sv, "utf-8"sv};                // UTF-8 variants
constexpr std::array ASCII_ENCODINGS = {"ascii"sv};                         // ASCII (7-bit)
constexpr std::array UTF16_ENCODINGS = {"utf16le"sv, "ucs2"sv, "ucs-2"sv};  // UTF-16 little-endian variants
constexpr std::array BASE64_ENCODINGS = {"base64"sv, "base64url"sv};        // Base64 variants
constexpr std::array LATIN_ENCODINGS = {"latin1"sv, "binary"sv};            // Latin1/binary encodings

[[nodiscard]] Result<bool> ValidateBuffer(const EtsEscompatArrayBuffer *buffer) noexcept
{
    if (buffer == nullptr) {
        return Err<PandaString>(PandaString("Buffer is null"));
    }
    if (buffer->WasDetached()) {
        return Err<PandaString>(PandaString("Buffer was detached"));
    }
    return true;
}
[[nodiscard]] Result<bool> ValidateIndices(int byteLength, int start, int end)
{
    if (start < 0 || start > byteLength) {
        return Err<PandaString>(PandaString("Start index is out of bounds"));
    }
    if (end < 0 || end > byteLength || end < start) {
        return Err<PandaString>(PandaString("End index is out of bounds"));
    }
    return true;
}
[[nodiscard]] PandaString GetEncoding(const PandaString *encodingObj) noexcept
{
    return encodingObj != nullptr ? *encodingObj : PandaString("utf8");
}
PandaVector<uint8_t> BytesFromString(std::string_view input)
{
    PandaVector<uint8_t> bytes;
    bytes.assign(input.begin(), input.end());
    return bytes;
}

PandaString StringFromBytes(const PandaVector<uint8_t> &bytes)
{
    return PandaString(bytes.begin(), bytes.end());
}

PandaVector<uint8_t> MaskBytes(std::string_view input, uint8_t mask)
{
    PandaVector<uint8_t> bytes;
    bytes.resize(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        bytes[i] = static_cast<uint8_t>(static_cast<unsigned char>(input[i]) & mask);
    }
    return bytes;
}

PandaString ConvertUtf8Encoding(const PandaVector<uint8_t> &bytes)
{
    return StringFromBytes(bytes);
}

PandaString ConvertUtf16Encoding(const PandaVector<uint8_t> &bytes)
{
    if (bytes.size() % K_UTF16_BYTES_PER_CHAR != 0) {
        return PandaString("Invalid UTF-16 byte sequence");
    }
    PandaString output;
    for (size_t i = 0; i < bytes.size(); i += K_UTF16_BYTES_PER_CHAR) {
        uint16_t ch = static_cast<uint16_t>(bytes[i]) |
                      static_cast<uint16_t>(static_cast<uint16_t>(bytes[i + 1]) << K_HIGH_BYTE_SHIFT);
        output.push_back(static_cast<char>(ch));
    }
    return output;
}

PandaString ConvertBase64Encoding(const PandaVector<uint8_t> &bytes, std::string_view encoding)
{
    PandaString output = base64::Encode(bytes);
    if (encoding == "base64url") {
        std::replace(output.begin(), output.end(), '+', '-');
        std::replace(output.begin(), output.end(), '/', '_');
        size_t lastValidCharPos = output.find_last_not_of('=');
        if (lastValidCharPos != PandaString::npos) {
            output.erase(lastValidCharPos + 1);
        }
    }
    return output;
}

PandaVector<uint8_t> ConvertUtf8ToBytes(std::string_view input)
{
    return BytesFromString(input);
}

PandaVector<uint8_t> ConvertAsciiToBytes(std::string_view input)
{
    return MaskBytes(input, K_ASCII_MASK);
}

PandaVector<uint8_t> ConvertLatinToBytes(std::string_view input)
{
    return MaskBytes(input, K_BYTE_MASK);
}

PandaString ConvertHexEncoding(const PandaVector<uint8_t> &bytes)
{
    PandaOStringStream oss;
    const int width = 2;
    for (uint8_t byte : bytes) {
        oss << std::hex << std::setw(width) << std::setfill('0') << static_cast<int>(byte);
    }
    return oss.str();
}

PandaString ConvertLatinEncoding(const PandaVector<uint8_t> &bytes)
{
    PandaString output;
    output.reserve(bytes.size());
    for (uint8_t byte : bytes) {
        output.push_back(static_cast<char>(byte));
    }
    return output;
}

PandaVector<uint8_t> ConvertUtf16ToBytes(std::string_view input)
{
    const size_t kUtf16Multiplier = 2;
    const size_t kShiftBy8 = 8U;
    PandaVector<uint8_t> bytes;
    bytes.resize(input.size() * kUtf16Multiplier);

    for (size_t i = 0; i < input.size(); ++i) {
        auto ch = static_cast<uint16_t>(static_cast<unsigned char>(input[i]));
        auto position = i * kUtf16Multiplier;
        bytes[position] = static_cast<uint8_t>(ch & K_BYTE_MASK);
        bytes[position + 1] = static_cast<uint8_t>(static_cast<unsigned int>(ch) >> kShiftBy8);
    }
    return bytes;
}

Result<PandaVector<uint8_t>> ConvertBase64ToBytes(const PandaString &input, std::string_view encoding)
{
    PandaString decoded;
    if (encoding == "base64url") {
        PandaString temp = input;
        std::replace(temp.begin(), temp.end(), '-', '+');
        std::replace(temp.begin(), temp.end(), '_', '/');
        decoded = base64::Decode(temp);
    } else {
        decoded = base64::Decode(input);
    }

    return BytesFromString(decoded);
}

Result<PandaVector<uint8_t>> ConvertHexToBytes(const PandaString &input)
{
    if (input.size() % K_HEX_PAIR_SIZE != 0) {
        return Err<PandaString>(PandaString("Hex string must have an even length"));
    }
    PandaVector<uint8_t> bytes;
    size_t bytesLength = input.size() / K_HEX_PAIR_SIZE;
    bytes.reserve(bytesLength);
    const size_t kHexStringLength = 3;
    for (size_t i = 0; i < input.size(); i += K_HEX_PAIR_SIZE) {
        std::array<char, kHexStringLength> hex = {input[i], input[i + 1], '\0'};
        char *endptr = nullptr;
        uint64_t value = std::strtoul(hex.data(), &endptr, HEX_BASE);
        if (*endptr != '\0') {
            return Err<PandaString>(PandaString("Invalid hex string"));
        }
        bytes.push_back(static_cast<uint8_t>(value));
    }
    return bytes;
}

Result<PandaVector<uint8_t>> ConvertStringToBytes(const PandaString &input, std::string_view encoding)
{
    if (std::find(UTF8_ENCODINGS.begin(), UTF8_ENCODINGS.end(), encoding) != UTF8_ENCODINGS.end()) {
        return ConvertUtf8ToBytes(input);
    }
    if (std::find(ASCII_ENCODINGS.begin(), ASCII_ENCODINGS.end(), encoding) != ASCII_ENCODINGS.end()) {
        return ConvertAsciiToBytes(input);
    }
    if (std::find(UTF16_ENCODINGS.begin(), UTF16_ENCODINGS.end(), encoding) != UTF16_ENCODINGS.end()) {
        return ConvertUtf16ToBytes(input);
    }
    if (std::find(BASE64_ENCODINGS.begin(), BASE64_ENCODINGS.end(), encoding) != BASE64_ENCODINGS.end()) {
        return ConvertBase64ToBytes(input, encoding);
    }
    if (std::find(LATIN_ENCODINGS.begin(), LATIN_ENCODINGS.end(), encoding) != LATIN_ENCODINGS.end()) {
        return ConvertLatinToBytes(input);
    }
    if (encoding == "hex") {
        return ConvertHexToBytes(input);
    }
    return Err<PandaString>(PandaString("Unsupported encoding: ") + PandaString(encoding));
}

Result<int32_t> CalculateStringBytesLength(std::string_view input, std::string_view encoding)
{
    const int32_t kUtf16Multiplier = 2;
    const int32_t kHexDivisor = 2;
    if (std::find(K_SINGLE_BYTE_ENCODINGS.begin(), K_SINGLE_BYTE_ENCODINGS.end(), encoding) !=
        K_SINGLE_BYTE_ENCODINGS.end()) {
        return static_cast<int32_t>(input.size());
    }
    if (std::find(K_DOUBLE_BYTE_ENCODINGS.begin(), K_DOUBLE_BYTE_ENCODINGS.end(), encoding) !=
        K_DOUBLE_BYTE_ENCODINGS.end()) {
        return static_cast<int32_t>(input.size() * kUtf16Multiplier);
    }
    if (encoding == "base64" || encoding == "base64url") {
        size_t len = input.size();
        size_t pad = ((len != 0U) && input.back() == '=') ? 1 : 0;
        size_t offsetTwo = 2;
        if ((pad != 0U) && len > 1 && input[len - offsetTwo] == '=') {
            ++pad;
        }
        size_t threeLength = len * 3;
        size_t s = threeLength / base64::K_BASE64_BLOCK_SIZE;
        if (s < pad) {
            return Err<PandaString>(PandaString("Invalid base64 string: ") + PandaString(input));
        }
        size_t size = s - pad;
        return static_cast<int32_t>(size);
    }
    if (encoding == "hex") {
        if (input.size() % kHexDivisor != 0) {
            return Err<PandaString>(PandaString("Hex string must have an even length"));
        }
        return static_cast<int32_t>(input.size() / kHexDivisor);
    }
    return Err<PandaString>(PandaString("Unsupported encoding: ") + PandaString(encoding));
}

}  // namespace encoding

}  // namespace ark::ets::intrinsics::helpers
