/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

// Autogenerated file by wrong_ctx_tests.rb script -- DO NOT EDIT!

#include "libabckit/include/c/abckit.h"
#include "libabckit/include/c/metadata_core.h"
#include "libabckit/include/c/extensions/arkts/metadata_arkts.h"
#include "libabckit/include/c/extensions/js/metadata_js.h"
#include "libabckit/include/c/ir_core.h"

#include "helpers/helpers.h"
#include "helpers/helpers_wrong_ctx.h"

#include <gtest/gtest.h>

namespace libabckit::test {

static auto g_isaApiDynamicImp = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitWrongCtxTestsIsaApiDynamicImpl1 : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateWideCallrange,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateWideCallrange)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateWideCallrange);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateGreatereq,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateGreatereq)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateGreatereq);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallarg0,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateCallarg0)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateCallarg0);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStglobalvar,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateStglobalvar)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateStglobalvar);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateInc,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateInc)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateInc);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateXor2,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateXor2)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateXor2);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateSetobjectwithproto,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateSetobjectwithproto)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateSetobjectwithproto);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCreateasyncgeneratorobj,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateCreateasyncgeneratorobj)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateCreateasyncgeneratorobj);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStownbyname,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateStownbyname)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateStownbyname);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateAsyncgeneratorreject,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateAsyncgeneratorreject)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateAsyncgeneratorreject);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateGetiterator,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateGetiterator)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateGetiterator);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateNoteq,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateNoteq)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateNoteq);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateIf,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateIf)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateIf);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDelobjprop,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateDelobjprop)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateDelobjprop);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdsuperbyname,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateLdsuperbyname)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateLdsuperbyname);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeStsendablevar,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateCallruntimeStsendablevar)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateCallruntimeStsendablevar);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateWideSupercallarrowrange,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateWideSupercallarrowrange)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateWideSupercallarrowrange);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateAsyncfunctionresolve,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateAsyncfunctionresolve)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateAsyncfunctionresolve);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeNotifyconcurrentresult,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateCallruntimeNotifyconcurrentresult)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateCallruntimeNotifyconcurrentresult);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeDefinesendableclass,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateCallruntimeDefinesendableclass)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateCallruntimeDefinesendableclass);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateWideStmodulevar,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateWideStmodulevar)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateWideStmodulevar);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateGreater,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateGreater)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateGreater);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateTrystglobalbyname,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateTrystglobalbyname)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateTrystglobalbyname);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateMod2,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateMod2)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateMod2);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLesseq,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateLesseq)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateLesseq);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallargs2,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateCallargs2)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateCallargs2);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeIsfalse,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateCallruntimeIsfalse)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateCallruntimeIsfalse);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDefinefieldbyname,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateDefinefieldbyname)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateDefinefieldbyname);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStownbyvaluewithnameset,
// abc-kind=NoABC, category=negative-file
TEST_F(LibAbcKitWrongCtxTestsIsaApiDynamicImpl1, iCreateStownbyvaluewithnameset)
{
    helpers_wrong_ctx::TestWrongCtx(g_isaApiDynamicImp->iCreateStownbyvaluewithnameset);
}

}  // namespace libabckit::test
