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

#include "regexp_replace.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PCRE2_CODE_UNIT_WIDTH 0
#include "pcre2.h"

#include "plugins/ets/stdlib/native/core/stdlib_ani_helpers.h"

#include <algorithm>
#include <array>
#include <memory>
#include <type_traits>
#include <utility>

namespace ark::ets::stdlib {
namespace {

struct SubstCalloutData {
    int32_t lastEndIndex = -1;
};

template <typename Block>
int UpdateLastEndIndex(Block *block, void *userData)
{
    auto *data = static_cast<SubstCalloutData *>(userData);
    if (block != nullptr && block->ovector != nullptr && data != nullptr) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        data->lastEndIndex = static_cast<int32_t>(block->ovector[1]);
    }
    return 0;
}

template <typename CharT>
struct SubstituteResult {
    bool valid = false;
    bool matched = false;
    int32_t lastEndIndex = -1;
    std::vector<CharT> output;
};

template <typename CharT>
// CC-OFFNXT(G.FUN.01, huge_method) solid logic
SubstituteResult<CharT> DoPcre2Substitute(void *codePtr, const CharT *input, ani_int inputLen, ani_int startOffset,
                                          uint32_t options, const CharT *replacement, ani_int replacementLen)
{
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    constexpr bool U8 = std::is_same_v<CharT, uint8_t>;
    using Ctx = std::conditional_t<U8, pcre2_match_context_8, pcre2_match_context_16>;

    SubstituteResult<CharT> result;
    auto deleter = +[](Ctx *c) {
        if constexpr (U8) {
            pcre2_match_context_free_8(c);
        } else {
            pcre2_match_context_free_16(c);
        }
    };
    std::unique_ptr<Ctx, decltype(deleter)> ctx(U8 ? reinterpret_cast<Ctx *>(pcre2_match_context_create_8(nullptr))
                                                   : reinterpret_cast<Ctx *>(pcre2_match_context_create_16(nullptr)),
                                                deleter);
    if (ctx == nullptr) {
        return result;
    }

    SubstCalloutData calloutData {};
    int rc = U8 ? pcre2_set_substitute_callout_8(reinterpret_cast<pcre2_match_context_8 *>(ctx.get()),
                                                 &UpdateLastEndIndex<pcre2_substitute_callout_block_8>, &calloutData)
                : pcre2_set_substitute_callout_16(reinterpret_cast<pcre2_match_context_16 *>(ctx.get()),
                                                  &UpdateLastEndIndex<pcre2_substitute_callout_block_16>, &calloutData);
    if (rc != 0) {
        return result;
    }

    auto sub = [codePtr, input, inputLen, startOffset, options, replacementLen, replacement, &ctx](CharT *outBuf,
                                                                                                   PCRE2_SIZE *outLen) {
        const auto inLen = static_cast<PCRE2_SIZE>(inputLen);
        const auto off = static_cast<PCRE2_SIZE>(startOffset);
        const auto rLen = static_cast<PCRE2_SIZE>(replacementLen);
        if constexpr (U8) {
            return pcre2_substitute_8(reinterpret_cast<pcre2_code_8 *>(codePtr), input, inLen, off, options, nullptr,
                                      reinterpret_cast<pcre2_match_context_8 *>(ctx.get()), replacement, rLen, outBuf,
                                      outLen);
        } else {
            return pcre2_substitute_16(reinterpret_cast<pcre2_code_16 *>(codePtr), input, inLen, off, options, nullptr,
                                       reinterpret_cast<pcre2_match_context_16 *>(ctx.get()), replacement, rLen, outBuf,
                                       outLen);
        }
    };

    PCRE2_SIZE outLength = 0;
    rc = sub(nullptr, &outLength);
    if (rc == 0) {
        result.valid = true;
        return result;
    }
    if (rc != PCRE2_ERROR_NOMEMORY) {
        return result;
    }

    std::vector<CharT> outputBuf(static_cast<size_t>(outLength));
    rc = sub(outputBuf.data(), &outLength);
    if (rc < 0) {
        return result;
    }
    result.valid = true;
    result.matched = true;
    result.lastEndIndex = calloutData.lastEndIndex;
    outputBuf.resize(static_cast<size_t>(outLength));
    result.output = std::move(outputBuf);
    return result;
}

template <typename CharT>
ani_string MakeResultString(ani_env *env, const std::vector<CharT> &out)
{
    if constexpr (std::is_same_v<CharT, uint8_t>) {
        return out.empty() ? CreateUtf8String(env, "", 0)
                           : CreateUtf8String(env, reinterpret_cast<const char *>(out.data()),
                                              static_cast<ani_size>(out.size()));
    } else {
        constexpr std::array<uint16_t, 1> EMPTY {0};
        return out.empty() ? CreateUtf16String(env, EMPTY.data(), 0)
                           : CreateUtf16String(env, out.data(), static_cast<ani_size>(out.size()));
    }
}

template <typename CharT>
// CC-OFFNXT(G.FUN.01, huge_method) solid logic
SubstituteResult<CharT> RunLiteralSubstitute(EtsRegExp &re, uint32_t matchFlags, ani_env *env, const ExecData &execData,
                                             ani_string replaceValue, InputExecutionKind ek)
{
    const uint32_t opts = matchFlags | (re.IsGlobal() ? PCRE2_SUBSTITUTE_GLOBAL : 0U) | PCRE2_SUBSTITUTE_LITERAL |
                          PCRE2_SUBSTITUTE_OVERFLOW_LENGTH;
    const auto inSize = static_cast<ani_int>(execData.inputSize);

    if constexpr (std::is_same_v<CharT, uint8_t>) {
        auto repl = ConvertFromAniString(env, replaceValue);
        ark::ets::ani::ScopedManagedCodeFix scope(env);
        RegExpStringAccessor pat(scope.ToInternalType(execData.pattern));
        if (!re.Compile(pat.GetDataUtf8(), static_cast<int>(execData.patternSize))) {
            return {};
        }
        RegExpStringAccessor in(scope.ToInternalType(execData.input));
        return DoPcre2Substitute<uint8_t>(re.GetCompiledRe8()->pcre2Code, in.GetDataUtf8(), inSize, execData.lastIndex,
                                          opts, reinterpret_cast<const uint8_t *>(repl.data()),
                                          static_cast<ani_int>(repl.size()));
    } else {
        std::vector<uint16_t> replStorage;
        std::vector<uint16_t> inStorage;
        ani_int replLen = 0;
        ark::ets::ani::ScopedManagedCodeFix scope(env);
        const uint16_t *inData = AcquireUtf16Input(scope, execData.input, static_cast<int32_t>(execData.inputSize),
                                                   ek == InputExecutionKind::LATIN1_TO_UTF16, inStorage);
        auto &patU16 = GetUtf16ScratchB();
        if (!CompileUtf16Pattern(re, scope, execData, patU16)) {
            return {};
        }
        const uint16_t *replData = AcquireUtf16Replacement(scope, replaceValue, replStorage, replLen);
        return DoPcre2Substitute<uint16_t>(re.GetCompiledRe16()->pcre2Code, inData, inSize, execData.lastIndex, opts,
                                           replData, replLen);
    }
}

template <typename CharT>
// CC-OFFNXT(G.FUN.01, huge_method) solid logic
ani_string ReplaceLiteralCore(EtsRegExp &re, uint32_t matchFlags, ani_env *env, ani_object regexp, ani_field lif,
                              bool gOrS, const ExecData &execData, ani_string replaceValue, InputExecutionKind ek)
{
    auto r = RunLiteralSubstitute<CharT>(re, matchFlags, env, execData, replaceValue, ek);
    if (!r.valid) {
        return nullptr;
    }
    if (!r.matched) {
        if (gOrS) {
            SetLastIndex(env, regexp, lif, 0);
        }
        return nullptr;
    }
    if (gOrS && r.lastEndIndex >= 0) {
        SetLastIndex(env, regexp, lif, r.lastEndIndex);
    }
    return MakeResultString<CharT>(env, r.output);
}

template <typename CharT>
// CC-OFFNXT(G.FUN.01, huge_method) solid logic
ani_status AdvanceReplaceLoop(EtsRegExp &re, const CharT *input, int32_t inputSize, bool unicode, ani_env *env,
                              ani_object regexp, ani_field lif, ani_class resCls, ani_method resCtor,
                              const RegExpExecResult &er, bool gOrS, int32_t &lastIndex,
                              std::vector<ani_object> &results, bool *shouldContinue)
{
    *shouldContinue = false;

    ani_object resultObj = nullptr;
    ANI_RETURN_ON_PENDING_ERROR(CreateRegExpResultObject(env, resCls, resCtor, er, &resultObj));
    results.push_back(resultObj);
    if (gOrS) {
        SetLastIndex(env, regexp, lif, er.endIndex);
    }
    if (!re.IsGlobal()) {
        return ANI_OK;
    }
    lastIndex = er.endIndex;
    if (er.index == er.endIndex) {
        lastIndex = AdvanceIndex(input, inputSize, lastIndex, unicode);
        if (gOrS) {
            SetLastIndex(env, regexp, lif, lastIndex);
        }
    }
    *shouldContinue = true;
    return ANI_OK;
}

template <typename CharT>
// CC-OFFNXT(G.FUN.01, huge_method) solid logic
ani_array RunReplaceLoop(EtsRegExp &re, uint32_t matchFlags, const CharT *input, int32_t inputSize, bool unicode,
                         ani_env *env, ani_object regexp, ani_field lif, ani_class resCls, ani_method resCtor,
                         const ExecData &execData)
{
    std::vector<ani_object> results;
    const bool gOrS = re.IsGlobal() || re.IsSticky();
    int32_t lastIndex = (!gOrS || execData.lastIndex < 0) ? 0 : execData.lastIndex;
    if (static_cast<size_t>(lastIndex) > execData.inputSize) {
        if (gOrS) {
            SetLastIndex(env, regexp, lif, 0);
        }
        ani_array out = nullptr;
        ani_status status = CreateResultArray(env, results, &out);
        return status == ANI_OK ? out : nullptr;
    }
    while (lastIndex <= inputSize) {
        auto er = ExecuteOnce(re, matchFlags, input, inputSize, lastIndex);
        if (!er.isSuccess) {
            if (gOrS) {
                SetLastIndex(env, regexp, lif, 0);
            }
            break;
        }
        bool shouldContinue = false;
        ani_status status = AdvanceReplaceLoop(re, input, inputSize, unicode, env, regexp, lif, resCls, resCtor, er,
                                               gOrS, lastIndex, results, &shouldContinue);
        if (status != ANI_OK) {
            return nullptr;
        }
        if (!shouldContinue) {
            break;
        }
    }
    ani_array out = nullptr;
    ani_status status = CreateResultArray(env, results, &out);
    return status == ANI_OK ? out : nullptr;
}

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
ani_array RunReplaceNativeImpl(EtsRegExp &re, const ExecData &execData, int32_t inputSize, ani_env *env,
                               ani_object regexp, ani_field lif, ani_class resCls, ani_method resCtor,
                               InputExecutionKind ek, uint32_t matchFlags)
{
    return PrepareInputAndRun<ani_array>(
        re, execData, env, ek,
        [&re, matchFlags, inputSize, env, regexp, lif, resCls, resCtor, execData](const uint8_t *p) -> ani_array {
            return RunReplaceLoop<uint8_t>(re, matchFlags, p, inputSize, false, env, regexp, lif, resCls, resCtor,
                                           execData);
        },
        [&re, matchFlags, inputSize, env, regexp, lif, resCls, resCtor, execData](const uint16_t *p) -> ani_array {
            return RunReplaceLoop<uint16_t>(re, matchFlags, p, inputSize, re.HasUnicodeOrUnicodeSetsFlag(), env, regexp,
                                            lif, resCls, resCtor, execData);
        });
}

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
ani_string RunReplaceLiteralNativeImpl(EtsRegExp &re, ExecData execData, ani_env *env, ani_object regexp, ani_field lif,
                                       ani_string replaceValue, ani_string fallbackValue, InputExecutionKind ek,
                                       uint32_t matchFlags, bool gOrS)
{
    if (!gOrS || execData.lastIndex < 0) {
        execData.lastIndex = 0;
    }
    if (static_cast<size_t>(execData.lastIndex) > execData.inputSize) {
        if (gOrS) {
            SetLastIndex(env, regexp, lif, 0);
        }
        return fallbackValue;
    }
    switch (ek) {
        case InputExecutionKind::LATIN1_DIRECT:
            return ReplaceLiteralCore<uint8_t>(re, matchFlags, env, regexp, lif, gOrS, execData, replaceValue,
                                               InputExecutionKind::LATIN1_DIRECT);
        case InputExecutionKind::UTF16_DIRECT:
        case InputExecutionKind::LATIN1_TO_UTF16:
            return ReplaceLiteralCore<uint16_t>(re, matchFlags, env, regexp, lif, gOrS, execData, replaceValue, ek);
        default:
            UNREACHABLE();
    }
}

}  // namespace

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
ani_array ReplaceNativeImpl(ani_env *env, ani_object regexp, ani_string pattern, ani_string flags, ani_string str,
                            ani_int patternSize, ani_int strSize, ani_int lastIndex, ani_boolean requiresUtf16Execution)
{
    ExecData execData = MakeExecData(env, pattern, str, flags, patternSize, strSize, lastIndex, requiresUtf16Execution);
    EtsRegExp re(env);
    re.SetFlags(ConvertFromAniString(env, execData.flags));

    ani_method resCtor;
    ANI_FATAL_IF_ERROR(env->Class_FindMethod(refs::g_regexpExecArrayClass, "<ctor>", ":", &resCtor));
    ani_field lif = nullptr;
    ANI_FATAL_IF_ERROR(FindLastIndexField(env, &lif));

    ani_array result = RunReplaceNativeImpl(re, execData, static_cast<int32_t>(strSize), env, regexp, lif,
                                            refs::g_regexpExecArrayClass, resCtor, SelectExecutionKind(re, execData),
                                            re.GetMatchFlags());
    if (re.HasCompiledRe()) {
        re.Destroy();
    }
    return result;
}

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
ani_string ReplaceLiteralNativeImpl(ani_env *env, ani_object regexp, ani_string pattern, ani_string flags,
                                    ani_string str, ani_string replaceValue, ani_int patternSize, ani_int strSize,
                                    ani_int lastIndex, ani_boolean requiresUtf16Execution)
{
    ExecData execData = MakeExecData(env, pattern, str, flags, patternSize, strSize, lastIndex, requiresUtf16Execution);
    EtsRegExp re(env);
    re.SetFlags(ConvertFromAniString(env, execData.flags));
    ani_field lif = nullptr;
    ANI_FATAL_IF_ERROR(FindLastIndexField(env, &lif));

    const bool gOrS = re.IsGlobal() || re.IsSticky();
    ani_string result = RunReplaceLiteralNativeImpl(re, execData, env, regexp, lif, replaceValue, str,
                                                    SelectExecutionKind(re, execData), re.GetMatchFlags(), gOrS);
    if (re.HasCompiledRe()) {
        re.Destroy();
    }
    if (result != nullptr) {
        return result;
    }

    ani_boolean hasUnhandledError = ANI_FALSE;
    ANI_FATAL_IF_ERROR(env->ExistUnhandledError(&hasUnhandledError));
    if (hasUnhandledError == ANI_TRUE) {
        return nullptr;
    }
    return str;
}

}  // namespace ark::ets::stdlib
