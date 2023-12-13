/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "base_options.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/napi/ets_napi.h"

namespace ark::ets::test {

// NOLINTBEGIN(google-runtime-int)
extern "C" ets_long SkoalaCreateRedrawerPeer([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                             [[maybe_unused]] ets_object object /*any*/)
{
    return 1;
}
extern "C" ets_long SkoalaGetFrame([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                   [[maybe_unused]] ets_long peer /*KNativePointer*/, [[maybe_unused]] ets_int a,
                                   [[maybe_unused]] ets_int b)
{
    return 1;
}
extern "C" ets_long SkoalaInitRedrawer([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                       [[maybe_unused]] ets_int width, [[maybe_unused]] ets_int height,
                                       [[maybe_unused]] ets_float scale,
                                       [[maybe_unused]] ets_long peer /*KNativePointer*/,
                                       [[maybe_unused]] ets_long frame /*KNativePointer*/)
{
    return 1;
}
extern "C" ets_long SkoalaPaint1nMake([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass)
{
    return 1;
}
extern "C" ets_long SkoalaPictureRecorder1nBeginRecording(
    [[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass, [[maybe_unused]] ets_long ptr /*KNativePointer*/,
    [[maybe_unused]] ets_float left, [[maybe_unused]] ets_float top, [[maybe_unused]] ets_float right,
    [[maybe_unused]] ets_float bottom)
{
    return 1;
}
extern "C" ets_long SkoalaPictureRecorder1nFinishRecordingAsDrawable([[maybe_unused]] EtsEnv *env,
                                                                     [[maybe_unused]] ets_class klass,
                                                                     [[maybe_unused]] ets_long ptr /*KNativePointer*/)
{
    return 1;
}
extern "C" ets_long SkoalaPictureRecorder1nFinishRecordingAsPictureWithCull(
    [[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass, [[maybe_unused]] ets_long ptr /*KNativePointer*/,
    [[maybe_unused]] ets_float left, [[maybe_unused]] ets_float top, [[maybe_unused]] ets_float right,
    [[maybe_unused]] ets_float bottom)
{
    return 1;
}
extern "C" ets_long SkoalaPictureRecorder1nMake([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass)
{
    return 1;
}
extern "C" ets_long SkoalaParagraphParagraphBuilder1nMake(
    [[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
    [[maybe_unused]] ets_long paragraphStylePtr /*KNativePointer*/,
    [[maybe_unused]] ets_long fontCollectionPtr /*KNativePointer*/)
{
    return 1;
}
extern "C" ets_long SkoalaParagraphParagraphBuilder1nGetFinalizer([[maybe_unused]] EtsEnv *env,
                                                                  [[maybe_unused]] ets_class klass)
{
    return 1;
}
extern "C" ets_long SkoalaParagraphParagraphBuilder1nBuild([[maybe_unused]] EtsEnv *env,
                                                           [[maybe_unused]] ets_class klass,
                                                           [[maybe_unused]] ets_long ptr /*KNativePointer*/)
{
    return 1;
}
extern "C" ets_long SkoalaParagraphFontCollection1nMake([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass)
{
    return 1;
}
extern "C" ets_long SkoalaFontMgr1nDefault([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass)
{
    return 1;
}
extern "C" ets_long SkoalaParagraphParagraphStyle1nMake([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass)
{
    return 1;
}
extern "C" ets_long SkoalaParagraphTextStyle1nMake([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass)
{
    return 1;
}
extern "C" ets_long SkoalaManagedString1nMake([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                              [[maybe_unused]] ets_byteArray textStr /*KStringPtr*/)
{
    return 1;
}
extern "C" ets_long SkoalaPaint1nGetFinalizer([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass)
{
    return 1;
}
extern "C" ets_long SkoalaImplRefCntGetFinalizer([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass)
{
    return 1;
}
extern "C" ets_long SkoalaParagraphParagraphStyle1nGetFinalizer([[maybe_unused]] EtsEnv *env,
                                                                [[maybe_unused]] ets_class klass)
{
    return 1;
}
extern "C" ets_long SkoalaParagraphTextStyle1nGetFinalizer([[maybe_unused]] EtsEnv *env,
                                                           [[maybe_unused]] ets_class klass)
{
    return 1;
}
extern "C" ets_long SkoalaParagraphParagraph1nGetFinalizer([[maybe_unused]] EtsEnv *env,
                                                           [[maybe_unused]] ets_class klass)
{
    return 1;
}
extern "C" ets_long SkoalaManagedString1nGetFinalizer([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass)
{
    return 1;
}
extern "C" ets_long GetPeerFactory([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass)
{
    return 1;
}
extern "C" ets_long GetEngine([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass)
{
    return 1;
}
extern "C" ets_int SkoalaGetFrameWidth([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                       [[maybe_unused]] ets_long peer /*KNativePointer*/,
                                       [[maybe_unused]] ets_long frame /*KNativePointer*/)
{
    return 1;
}
extern "C" ets_int SkoalaGetFrameHeight([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                        [[maybe_unused]] ets_long peer /*KNativePointer*/,
                                        [[maybe_unused]] ets_long frame /*KNativePointer*/)
{
    return 1;
}
extern "C" ets_int SkoalaCanvas1nSave([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                      [[maybe_unused]] ets_long ptr /*KNativePointer*/)
{
    return 1;
}
extern "C" void SkoalaDrawPicture([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                  [[maybe_unused]] ets_long picture /*KNativePointer*/,
                                  [[maybe_unused]] ets_long data /*KNativePointer*/,
                                  [[maybe_unused]] ets_object cb /*any*/, [[maybe_unused]] ets_boolean sync)
{
}
extern "C" void SkoalaProvidePeerFactory([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                         [[maybe_unused]] ets_long func /*KNativePointer*/,
                                         [[maybe_unused]] ets_long arg /*KNativePointer*/)
{
}
extern "C" void SkoalaSetPlatformApi([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                     [[maybe_unused]] ets_object api /*any*/)
{
}
extern "C" void SkoalaCanvas1nDrawDrawable([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                           [[maybe_unused]] ets_long ptr /*KNativePointer*/,
                                           [[maybe_unused]] ets_long drawablePtr /*KNativePointer*/,
                                           [[maybe_unused]] ets_long matrixArr /*KFloatPtr*/)
{
}
extern "C" void SkoalaCanvas1nRestore([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                      [[maybe_unused]] ets_long ptr /*KNativePointer*/)
{
}
extern "C" void SkoalaPaint1nSetColor([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                      [[maybe_unused]] ets_long ptr /*KNativePointer*/, [[maybe_unused]] ets_int color)
{
}
extern "C" void SkoalaCanvas1nDrawOval([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                       [[maybe_unused]] ets_long canvasPtr /*KNativePointer*/,
                                       [[maybe_unused]] ets_float left, [[maybe_unused]] ets_float top,
                                       [[maybe_unused]] ets_float right, [[maybe_unused]] ets_float bottom,
                                       [[maybe_unused]] ets_long paintPtr /*KNativePointer*/)
{
}
extern "C" void SkoalaParagraphParagraphBuilder1nPushStyle([[maybe_unused]] EtsEnv *env,
                                                           [[maybe_unused]] ets_class klass,
                                                           [[maybe_unused]] ets_long ptr /*KNativePointer*/,
                                                           [[maybe_unused]] ets_long textStylePtr /*KNativePointer*/)
{
}
extern "C" void SkoalaParagraphParagraphBuilder1nAddText([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                                         [[maybe_unused]] ets_long ptr /*KNativePointer*/,
                                                         [[maybe_unused]] ets_long textString /*KStringPtr*/)
{
}
extern "C" void SkoalaParagraphFontCollection1nSetDefaultFontManager(
    [[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass, [[maybe_unused]] ets_long ptr /*KNativePointer*/,
    [[maybe_unused]] ets_long fontManagerPtr /*KNativePointer*/,
    [[maybe_unused]] ets_long defaultFamilyNameStr /*KStringPtr*/)
{
}
extern "C" void SkoalaParagraphParagraph1nLayout([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                                 [[maybe_unused]] ets_long ptr /*KNativePointer*/,
                                                 [[maybe_unused]] ets_float width)
{
}
extern "C" void SkoalaParagraphParagraph1nPaint([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                                [[maybe_unused]] ets_long ptr /*KNativePointer*/,
                                                [[maybe_unused]] ets_long canvasPtr /*KNativePointer*/,
                                                [[maybe_unused]] ets_float x, [[maybe_unused]] ets_float y)
{
}
extern "C" void SkoalaParagraphTextStyle1nSetColor([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                                   [[maybe_unused]] ets_long ptr /*KNativePointer*/,
                                                   [[maybe_unused]] ets_int color)
{
}
extern "C" void SkoalaParagraphTextStyle1nSetFontSize([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                                      [[maybe_unused]] ets_long ptr /*KNativePointer*/,
                                                      [[maybe_unused]] ets_float size)
{
}
extern "C" void SkoalaImplManagedInvokeFinalizer([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                                 [[maybe_unused]] ets_long finalizer /*KNativePointer*/,
                                                 [[maybe_unused]] ets_long obj /*KNativePointer*/)
{
}
extern "C" void SkoalaEnqueueRun([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                 [[maybe_unused]] ets_long redrawerPeerPtr /*KNativePointer*/)
{
}
// NOLINTEND(google-runtime-int)

class EtsVMConfingNapi : public testing::Test {
public:
    bool CreateRuntime(const std::string &stdlibAbc, const std::string &pathAbc, bool enableJit,
                       const std::string &pathAn = "")
    {
        std::vector<EtsVMOption> options = {{EtsOptionType::ETS_LOG_LEVEL, "info"},
                                            {EtsOptionType::ETS_BOOT_FILE, stdlibAbc.c_str()},
                                            {EtsOptionType::ETS_BOOT_FILE, pathAbc.c_str()},
                                            {EtsOptionType::ETS_ARK_FILE, pathAbc.c_str()},
                                            {EtsOptionType::ETS_GC_TRIGGER_TYPE, "heap-trigger"},
                                            {enableJit ? EtsOptionType::ETS_JIT : EtsOptionType::ETS_NO_JIT, nullptr}};
        if (!pathAn.empty()) {
            options.emplace_back(EtsVMOption {EtsOptionType::ETS_AOT, nullptr});
            options.emplace_back(EtsVMOption {EtsOptionType::ETS_AOT_FILE, pathAn.c_str()});
        }

        EtsVMInitArgs args = {ETS_NAPI_VERSION_1_0, static_cast<ets_int>(options.size()), options.data()};
        return ETS_CreateVM(&vm_, &env_, &args) == ETS_OK;
    }

    bool InitExports()
    {
        const std::array<EtsNativeMethod, 43> impls = {
            {{"_skoala_createRedrawerPeer", nullptr, reinterpret_cast<void *>(SkoalaCreateRedrawerPeer)},
             {"_skoala_drawPicture", nullptr, reinterpret_cast<void *>(SkoalaDrawPicture)},
             {"_skoala_getFrame", nullptr, reinterpret_cast<void *>(SkoalaGetFrame)},
             {"_skoala_getFrameWidth", nullptr, reinterpret_cast<void *>(SkoalaGetFrameWidth)},
             {"_skoala_getFrameHeight", nullptr, reinterpret_cast<void *>(SkoalaGetFrameHeight)},
             {"_skoala_initRedrawer", nullptr, reinterpret_cast<void *>(SkoalaInitRedrawer)},
             {"_skoala_providePeerFactory", nullptr, reinterpret_cast<void *>(SkoalaProvidePeerFactory)},
             {"_skoala_setPlatformAPI", nullptr, reinterpret_cast<void *>(SkoalaSetPlatformApi)},
             {"_skoala_Canvas__1nDrawDrawable", nullptr, reinterpret_cast<void *>(SkoalaCanvas1nDrawDrawable)},
             {"_skoala_Canvas__1nRestore", nullptr, reinterpret_cast<void *>(SkoalaCanvas1nRestore)},
             {"_skoala_Canvas__1nSave", nullptr, reinterpret_cast<void *>(SkoalaCanvas1nSave)},
             {"_skoala_Paint__1nMake", nullptr, reinterpret_cast<void *>(SkoalaPaint1nMake)},
             {"_skoala_Paint__1nSetColor", nullptr, reinterpret_cast<void *>(SkoalaPaint1nSetColor)},
             {"_skoala_PictureRecorder__1nBeginRecording", nullptr,
              reinterpret_cast<void *>(SkoalaPictureRecorder1nBeginRecording)},
             {"_skoala_PictureRecorder__1nFinishRecordingAsDrawable", nullptr,
              reinterpret_cast<void *>(SkoalaPictureRecorder1nFinishRecordingAsDrawable)},
             {"_skoala_PictureRecorder__1nFinishRecordingAsPictureWithCull", nullptr,
              reinterpret_cast<void *>(SkoalaPictureRecorder1nFinishRecordingAsPictureWithCull)},
             {"_skoala_PictureRecorder__1nMake", nullptr, reinterpret_cast<void *>(SkoalaPictureRecorder1nMake)},
             {"_skoala_Canvas__1nDrawOval", nullptr, reinterpret_cast<void *>(SkoalaCanvas1nDrawOval)},
             {"_skoala_paragraph_ParagraphBuilder__1nMake", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphParagraphBuilder1nMake)},
             {"_skoala_paragraph_ParagraphBuilder__1nGetFinalizer", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphParagraphBuilder1nGetFinalizer)},
             {"_skoala_paragraph_ParagraphBuilder__1nPushStyle", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphParagraphBuilder1nPushStyle)},
             {"_skoala_paragraph_ParagraphBuilder__1nAddText", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphParagraphBuilder1nAddText)},
             {"_skoala_paragraph_ParagraphBuilder__1nBuild", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphParagraphBuilder1nBuild)},
             {"_skoala_paragraph_FontCollection__1nMake", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphFontCollection1nMake)},
             {"_skoala_paragraph_FontCollection__1nSetDefaultFontManager", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphFontCollection1nSetDefaultFontManager)},
             {"_skoala_FontMgr__1nDefault", nullptr, reinterpret_cast<void *>(SkoalaFontMgr1nDefault)},
             {"_skoala_paragraph_ParagraphStyle__1nMake", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphParagraphStyle1nMake)},
             {"_skoala_paragraph_Paragraph__1nLayout", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphParagraph1nLayout)},
             {"_skoala_paragraph_Paragraph__1nPaint", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphParagraph1nPaint)},
             {"_skoala_paragraph_TextStyle__1nMake", nullptr, reinterpret_cast<void *>(SkoalaParagraphTextStyle1nMake)},
             {"_skoala_paragraph_TextStyle__1nSetColor", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphTextStyle1nSetColor)},
             {"_skoala_paragraph_TextStyle__1nSetFontSize", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphTextStyle1nSetFontSize)},
             {"_skoala_ManagedString__1nMake", nullptr, reinterpret_cast<void *>(SkoalaManagedString1nMake)},
             {"_skoala_Paint__1nGetFinalizer", nullptr, reinterpret_cast<void *>(SkoalaPaint1nGetFinalizer)},
             {"_skoala_impl_Managed__invokeFinalizer", nullptr,
              reinterpret_cast<void *>(SkoalaImplManagedInvokeFinalizer)},
             {"_skoala_impl_RefCnt__getFinalizer", nullptr, reinterpret_cast<void *>(SkoalaImplRefCntGetFinalizer)},
             {"_skoala_paragraph_ParagraphStyle__1nGetFinalizer", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphParagraphStyle1nGetFinalizer)},
             {"_skoala_paragraph_TextStyle__1nGetFinalizer", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphTextStyle1nGetFinalizer)},
             {"_skoala_paragraph_Paragraph__1nGetFinalizer", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphParagraph1nGetFinalizer)},
             {"_skoala_ManagedString__1nGetFinalizer", nullptr,
              reinterpret_cast<void *>(SkoalaManagedString1nGetFinalizer)},
             {"_getPeerFactory", nullptr, reinterpret_cast<void *>(GetPeerFactory)},
             {"_getEngine", nullptr, reinterpret_cast<void *>(GetEngine)},
             {"_skoala_enqueue_run", nullptr, reinterpret_cast<void *>(SkoalaEnqueueRun)}}};

        auto globalClass = env_->FindClass("ETSGLOBAL");
        if (globalClass == nullptr) {
            return false;
        }

        return env_->RegisterNatives(globalClass, impls.data(), impls.size()) == ETS_OK;
    }

    bool DestroyRuntime() const
    {
        return vm_->DestroyEtsVM() == ETS_OK;
    }

    int ExecuteMain() const
    {
        auto klass = env_->FindClass("ETSGLOBAL");
        auto method = env_->GetStaticp_method(klass, "main", ":I");
        return env_->CallStaticIntMethod(klass, method);  // NOLINT(cppcoreguidelines-pro-type-vararg)
    }

private:
    EtsVM *vm_ {};
    EtsEnv *env_ {};
};

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
TEST_F(EtsVMConfingNapi, PeasINT)
{
    std::string stdlibAbc;
    std::string pathAbc;

#if defined(STDLIB_ABC) && defined(PATH_ABC)
#define ETS_UNIT_STRING_STEP(s) #s
#define ETS_UNIT_STRING(s) ETS_UNIT_STRING_STEP(s)
    stdlibAbc = ETS_UNIT_STRING(STDLIB_ABC);
    pathAbc = ETS_UNIT_STRING(PATH_ABC);
#undef ETS_UNIT_STRING
#undef ETS_UNIT_STRING_STEP
#endif

    ASSERT_FALSE(stdlibAbc.empty());
    ASSERT_FALSE(pathAbc.empty());

    ASSERT_TRUE(CreateRuntime(stdlibAbc, pathAbc, false));
    EXPECT_TRUE(InitExports());
    EXPECT_TRUE(ExecuteMain() == 0);
    ASSERT_TRUE(DestroyRuntime());
}

TEST_F(EtsVMConfingNapi, PeasJIT)
{
    std::string stdlibAbc;
    std::string pathAbc;

#if defined(STDLIB_ABC) && defined(PATH_ABC)
#define ETS_UNIT_STRING_STEP(s) #s
#define ETS_UNIT_STRING(s) ETS_UNIT_STRING_STEP(s)
    stdlibAbc = ETS_UNIT_STRING(STDLIB_ABC);
    pathAbc = ETS_UNIT_STRING(PATH_ABC);
#undef ETS_UNIT_STRING
#undef ETS_UNIT_STRING_STEP
#endif

    ASSERT_FALSE(stdlibAbc.empty());
    ASSERT_FALSE(pathAbc.empty());

    ASSERT_TRUE(CreateRuntime(stdlibAbc, pathAbc, true));
    EXPECT_TRUE(InitExports());
    EXPECT_TRUE(ExecuteMain() == 0);
    ASSERT_TRUE(DestroyRuntime());
}

TEST_F(EtsVMConfingNapi, PeasAOT)
{
#ifdef HOST_CROSSCOMPILING
    GTEST_SKIP();
#endif

    std::string stdlibAbc;
    std::string pathAbc;
    std::string pathAn;

#if defined(STDLIB_ABC) && defined(PATH_ABC)
#define ETS_UNIT_STRING_STEP(s) #s
#define ETS_UNIT_STRING(s) ETS_UNIT_STRING_STEP(s)
    stdlibAbc = ETS_UNIT_STRING(STDLIB_ABC);
    pathAbc = ETS_UNIT_STRING(PATH_ABC);
    pathAn = ETS_UNIT_STRING(PATH_AN_ARK);
#undef ETS_UNIT_STRING
#undef ETS_UNIT_STRING_STEP
#endif

    ASSERT_FALSE(stdlibAbc.empty());
    ASSERT_FALSE(pathAbc.empty());
    ASSERT_FALSE(pathAn.empty());

    ASSERT_TRUE(CreateRuntime(stdlibAbc, pathAbc, false, pathAn));
    EXPECT_TRUE(InitExports());
    EXPECT_TRUE(ExecuteMain() == 0);
    ASSERT_TRUE(DestroyRuntime());
}

TEST_F(EtsVMConfingNapi, PeasLLVMAOT)
{
#ifdef HOST_CROSSCOMPILING
    GTEST_SKIP();
#endif

#ifndef PANDA_LLVM_AOT
    GTEST_SKIP();
#endif

    std::string stdlibAbc;
    std::string pathAbc;
    std::string pathAn;

#if defined(STDLIB_ABC) && defined(PATH_ABC)
#define ETS_UNIT_STRING_STEP(s) #s
#define ETS_UNIT_STRING(s) ETS_UNIT_STRING_STEP(s)
    stdlibAbc = ETS_UNIT_STRING(STDLIB_ABC);
    pathAbc = ETS_UNIT_STRING(PATH_ABC);
    pathAn = ETS_UNIT_STRING(PATH_AN_LLVM);
#undef ETS_UNIT_STRING
#undef ETS_UNIT_STRING_STEP
#endif

    ASSERT_FALSE(stdlibAbc.empty());
    ASSERT_FALSE(pathAbc.empty());
    ASSERT_FALSE(pathAn.empty());

    ASSERT_TRUE(CreateRuntime(stdlibAbc, pathAbc, false, pathAn));
    EXPECT_TRUE(InitExports());
    EXPECT_TRUE(ExecuteMain() == 0);
    ASSERT_TRUE(DestroyRuntime());
}
// NOLINTEND(cppcoreguidelines-macro-usage)

}  // namespace ark::ets::test
