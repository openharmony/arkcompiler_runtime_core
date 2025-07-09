/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include <cstddef>
#include <cstring>
#include <limits>

#include "include/object_header.h"
#include "objects/base_object.h"
#include "objects/string/base_string.h"
#include "objects/string/base_string-inl.h"

#include "include/coretypes/line_string.h"
#include "libpandabase/utils/utf.h"
#include "libpandabase/utils/hash.h"
#include "libpandabase/utils/span.h"
#include "runtime/arch/memory_helpers.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/runtime.h"
#include "runtime/handle_base-inl.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/include/coretypes/line_string.h"

namespace ark::coretypes {

/* static */
String *String::CreateFromMUtf8(const uint8_t *mutf8Data, size_t mutf8Length, uint32_t utf16Length,
                                bool canBeCompressed, const LanguageContext &ctx, PandaVM *vm, bool movable,
                                bool pinned)
{
    return String::Cast(
        LineString::CreateFromMUtf8(mutf8Data, mutf8Length, utf16Length, canBeCompressed, ctx, vm, movable, pinned));
}

/* static */
String *String::CreateFromMUtf8(const uint8_t *mutf8Data, uint32_t utf16Length, bool canBeCompressed,
                                const LanguageContext &ctx, PandaVM *vm, bool movable, bool pinned)
{
    return String::Cast(LineString::CreateFromMUtf8(mutf8Data, utf16Length, canBeCompressed, ctx, vm, movable, pinned));
}

/* static */
String *String::CreateFromMUtf8(const uint8_t *mutf8Data, uint32_t utf16Length, const LanguageContext &ctx, PandaVM *vm,
                                bool movable, bool pinned)
{
    return String::Cast(LineString::CreateFromMUtf8(mutf8Data, utf16Length, ctx, vm, movable, pinned));
}

/* static */
String *String::CreateFromMUtf8(const uint8_t *mutf8Data, const LanguageContext &ctx, PandaVM *vm, bool movable,
                                bool pinned)
{
    return String::Cast(LineString::CreateFromMUtf8(mutf8Data, ctx, vm, movable, pinned));
}

/* static */
String *String::CreateFromMUtf8(const uint8_t *mutf8Data, uint32_t mutf8Length, uint32_t utf16Length,
                                const LanguageContext &ctx, PandaVM *vm, bool movable, bool pinned)
{
    return String::Cast(LineString::CreateFromMUtf8(mutf8Data, mutf8Length, utf16Length, ctx, vm, movable, pinned));
}

/* static */
String *String::CreateFromUtf8(const uint8_t *utf8Data, uint32_t utf8Length, const LanguageContext &ctx, PandaVM *vm,
                               bool movable, bool pinned)
{
    return String::Cast(LineString::CreateFromUtf8(utf8Data, utf8Length, ctx, vm, movable, pinned));
}

/* static */
String *String::CreateFromUtf16(const uint16_t *utf16Data, uint32_t utf16Length, bool canBeCompressed,
                                const LanguageContext &ctx, PandaVM *vm, bool movable, bool pinned)
{
    return String::Cast(LineString::CreateFromUtf16(utf16Data, utf16Length, canBeCompressed, ctx, vm, movable, pinned));
}

/* static */
String *String::CreateFromUtf16(const uint16_t *utf16Data, uint32_t utf16Length, const LanguageContext &ctx,
                                PandaVM *vm, bool movable, bool pinned)
{
    return String::Cast(LineString::CreateFromUtf16(utf16Data, utf16Length, ctx, vm, movable, pinned));
}

/* static */
String *String::CreateEmptyString(const LanguageContext &ctx, PandaVM *vm)
{
    return String::Cast(LineString::CreateEmptyLineString(ctx, vm));
}

/* static */
String *String::CreateNewStringFromChars(uint32_t offset, uint32_t length, Array *chararray, const LanguageContext &ctx,
                                         PandaVM *vm)
{
    return String::Cast(LineString::CreateNewLineStringFromChars(offset, length, chararray, ctx, vm));
}

/* static */
String *String::CreateNewStringFromBytes(uint32_t offset, uint32_t length, uint32_t highByte, Array *bytearray,
                                         const LanguageContext &ctx, PandaVM *vm)
{
    return String::Cast(LineString::CreateNewLineStringFromBytes(offset, length, highByte, bytearray, ctx, vm));
}

/* static */
String *String::CreateFromString(String *str, const LanguageContext &ctx, PandaVM *vm)
{
    ASSERT(str != nullptr);
    // allocator may trig gc and move str, need to hold it
    auto thread = ManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<String> strHandle(thread, str);
    ASSERT(strHandle.GetPtr() != nullptr);
    auto string = String::AllocLineStringObject(strHandle->GetLength(), !strHandle->IsUtf16(), ctx, vm);
    if (string == nullptr) {
        return nullptr;
    }

    // retrive str after gc
    str = strHandle.GetPtr();

    // After memcpy we should have a full barrier, so this writes should happen-before barrier
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
    string->WriteData(str, 0, string->GetLength(), str->GetLength());
    TSAN_ANNOTATE_IGNORE_WRITES_END();
    // String is supposed to be a constant object, so all its data should be visible by all threads
    arch::FullMemoryBarrier();
    return string;
}

Array *String::GetChars(String *src, uint32_t start, uint32_t utf16Length, const LanguageContext &ctx)
{
    // allocator may trig gc and move 'src', need to hold it
    auto thread = ManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<String> strHandle(thread, src);
    auto *klass = Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::ARRAY_U16);
    Array *array = Array::Create(klass, utf16Length);
    if (array == nullptr) {
        return nullptr;
    }

    for (uint32_t i = 0; i < utf16Length; i++) {
        array->Set<uint16_t>(i, strHandle->At(start + i));
    }
    return array;
}

/* static */
String *String::AllocLineStringObject(size_t length, bool compressed, const LanguageContext &ctx, PandaVM *vm,
                                      bool movable, bool pinned)
{
    ASSERT(vm != nullptr);
    auto *thread = ManagedThread::GetCurrent();
    auto *stringClass =
        Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::LINE_STRING);
    size_t size = compressed ? panda::LineString::ComputeSizeUtf8(length) : panda::LineString::ComputeSizeUtf16(length);
    panda::BaseString *string =
        movable ? reinterpret_cast<panda::BaseString *>(
                      // CC-OFFNXT(G.FMT.06-CPP) project code style
                      vm->GetHeapManager()->AllocateObject(stringClass, size, DEFAULT_ALIGNMENT, thread,
                                                           mem::ObjectAllocatorBase::ObjMemInitPolicy::REQUIRE_INIT,
                                                           pinned))
                : reinterpret_cast<panda::BaseString *>(vm->GetHeapManager()->AllocateNonMovableObject(
                      // CC-OFFNXT(G.FMT.06-CPP) project code style
                      stringClass, size, DEFAULT_ALIGNMENT, thread,
                      mem::ObjectAllocatorBase::ObjMemInitPolicy::REQUIRE_INIT));
    if (string != nullptr) {
        // After setting length we should have a full barrier, so this write should happens-before barrier
        TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
        string->InitLengthAndFlags(length, compressed);
        string->SetRawHashcode(0);
        TSAN_ANNOTATE_IGNORE_WRITES_END();
        // Witout full memory barrier it is possible that architectures with weak memory order can try fetching string
        // legth before it's set
        arch::FullMemoryBarrier();
    }
    return String::Cast(string);
}

/* static */
String *FlatStringInfo::SlowFlatten(VMHandle<String> &str, const LanguageContext &ctx)
{
    ASSERT(str->IsSlicedString() || str->IsTreeString());
    PandaVM *vm = Runtime::GetCurrent()->GetPandaVM();

    uint32_t length = str->GetLength();
    bool compressed = str->IsUtf8();

    String *result = String::AllocLineStringObject(length, compressed, ctx, vm);
    if (result == nullptr) {
        return nullptr;
    }

    auto thread = ManagedThread::GetCurrent();
    VMHandle<String> resultHandle(thread, result);
    auto readBarrier = [](void *obj, size_t offset) {
        return reinterpret_cast<panda::BaseString *>(ObjectAccessor::GetObject(const_cast<const void *>(obj), offset));
    };

    if (compressed) {
        panda::BaseString::WriteToFlat(std::move(readBarrier), str->ToString(), resultHandle->GetDataUtf8Writable(),
                                       length);
    } else {
        panda::BaseString::WriteToFlat(std::move(readBarrier), str->ToString(), resultHandle->GetDataUtf16Writable(),
                                       length);
    }
    return resultHandle.GetPtr();
}

/* static */
String *String::FastSubUtf8String(String *src, uint32_t start, uint32_t length, const LanguageContext &ctx)
{
    PandaVM *vm = Runtime::GetCurrent()->GetPandaVM();
    VMHandle<String> srcHandle(ManagedThread::GetCurrent(), src);

    // 1. alloc dest line string
    auto lineStr = String::AllocLineStringObject(length, true, ctx, vm);
    if (lineStr == nullptr) {
        return nullptr;
    }
    VMHandle<String> lineStrHandle(ManagedThread::GetCurrent(), lineStr);

    // 2. flatten src string
    FlatStringInfo srcFlat = FlatStringInfo::FlattenAllString(srcHandle, ctx);

    // 3. copy it
    panda::common::Span<uint8_t> dest(lineStrHandle->GetDataUtf8Writable(), length);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    panda::common::Span<const uint8_t> source(srcFlat.GetDataUtf8() + start, length);
    panda::BaseString::MemCopyChars(dest, length, source, length);
    return lineStrHandle.GetPtr();
}

/* static */
String *String::FastSubUtf16String(String *src, uint32_t start, uint32_t length, const LanguageContext &ctx)
{
    // 1. judge can compressed or not
    VMHandle<String> srcHandle(ManagedThread::GetCurrent(), src);
    FlatStringInfo srcFlat = FlatStringInfo::FlattenAllString(srcHandle, ctx);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    bool canBeCompressed = panda::BaseString::CanBeCompressed(srcFlat.GetDataUtf16() + start, length);

    // 2. alloc dest line string
    PandaVM *vm = Runtime::GetCurrent()->GetPandaVM();
    auto lineStr = String::AllocLineStringObject(length, canBeCompressed, ctx, vm);
    if (lineStr == nullptr) {
        return nullptr;
    }
    VMHandle<String> lineStrHandle(ManagedThread::GetCurrent(), lineStr);

    // maybe happen GC,so get srcFlat again
    srcFlat = FlatStringInfo::FlattenAllString(srcHandle, ctx);

    if (canBeCompressed) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        panda::BaseString::CopyChars(lineStrHandle->GetDataUtf8Writable(), srcFlat.GetDataUtf16() + start, length);
    } else {
        uint32_t len = length * (sizeof(uint16_t) / sizeof(uint8_t));
        panda::common::Span<uint16_t> dest(lineStrHandle->GetDataUtf16Writable(), length);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        panda::common::Span<const uint16_t> source(srcFlat.GetDataUtf16() + start, length);
        panda::BaseString::MemCopyChars(dest, len, source, len);
    }
    return lineStrHandle.GetPtr();
}

/* static */
String *String::FastSubString(String *src, uint32_t start, uint32_t length, const LanguageContext &ctx, PandaVM *vm)
{
    if (!(Runtime::GetOptions().IsUseAllStrings())) {
        return String::Cast(LineString::FastSubLineString(String::Cast(src), start, length, ctx, vm));
    }

    ASSERT(src != nullptr);
    ASSERT((start + length) <= src->GetLength());
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(ManagedThread::GetCurrent());

    if (length == 0) {
        return reinterpret_cast<String *>(LineString::CreateEmptyLineString(ctx, vm));
    }

    if (start == 0 && length == src->GetLength()) {
        return src;
    }

    // no need to make sliced string if too short
    if (length < panda::SlicedString::MIN_SLICED_STRING_LENGTH) {
        if (src->IsUtf8()) {
            return FastSubUtf8String(src, start, length, ctx);
        }
        return FastSubUtf16String(src, start, length, ctx);
    }

    VMHandle<String> srcHandle(ManagedThread::GetCurrent(), src);
    // src is utf16 , substr if all ASCII chars , no need to slice it
    if (src->IsUtf16()) {
        FlatStringInfo srcFlat = FlatStringInfo::FlattenAllString(srcHandle, ctx);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        bool canBeCompressed = panda::BaseString::CanBeCompressed(srcFlat.GetDataUtf16() + start, length);
        if (canBeCompressed) {
            auto lineStr = AllocLineStringObject(length, canBeCompressed, ctx, vm);
            if (lineStr == nullptr) {
                return nullptr;
            }
            VMHandle<String> lineStrHandle(ManagedThread::GetCurrent(), lineStr);

            // maybe happen gc , get srcFlat again
            srcFlat = FlatStringInfo::FlattenAllString(srcHandle, ctx);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            panda::BaseString::CopyChars(lineStrHandle->GetDataUtf8Writable(), srcFlat.GetDataUtf16() + start, length);
            return lineStrHandle.GetPtr();
        }
    }
    return GetSlicedString(srcHandle.GetPtr(), start, length, ctx, vm);
}

/* static */
String *String::Concat(String *str1, String *str2, const LanguageContext &ctx, PandaVM *vm)
{
    if (!(Runtime::GetOptions().IsUseAllStrings())) {
        return String::Cast(LineString::Concat(String::Cast(str1), String::Cast(str2), ctx, vm));
    }

    ASSERT(str1 != nullptr);
    ASSERT(str2 != nullptr);
    // allocator may trig gc and move src, need to hold it
    auto thread = ManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<String> str1Handle(thread, str1);
    VMHandle<String> str2Handle(thread, str2);

    uint32_t length1 = str1Handle->GetLength();
    uint32_t length2 = str2Handle->GetLength();
    uint32_t newLength = length1 + length2;
    if (newLength == 0) {
        return reinterpret_cast<String *>(LineString::CreateEmptyLineString(ctx, vm));
    }
    if (length1 == 0) {
        return str2Handle.GetPtr();
    }
    if (length2 == 0) {
        return str1Handle.GetPtr();
    }

    // concat with tree string if too long
    bool compressed = String::GetCompressedStringsEnabled() && (str1Handle->IsUtf8()) && (str2Handle->IsUtf8());
    if (newLength >= panda::TreeString::MIN_TREE_STRING_LENGTH) {
        return CreateTreeString(str1Handle.GetPtr(), str2Handle.GetPtr(), newLength, compressed, ctx, vm);
    }

    // concat with line string if short
    ASSERT(str1Handle->IsLineString());
    ASSERT(str2Handle->IsLineString());
    return ConcatLineString(str1Handle, str2Handle, ctx, vm);
}

/* static */
String *String::ConcatLineString(VMHandle<String> &str1Handle, VMHandle<String> &str2Handle, const LanguageContext &ctx,
                                 PandaVM *vm)
{
    uint32_t length1 = str1Handle->GetLength();
    uint32_t length2 = str2Handle->GetLength();
    uint32_t newLength = length1 + length2;
    bool compressed = String::GetCompressedStringsEnabled() && (str1Handle->IsUtf8()) && (str2Handle->IsUtf8());

    auto newString = String::AllocLineStringObject(newLength, compressed, ctx, vm);
    if (UNLIKELY(newString == nullptr)) {
        return nullptr;
    }
    VMHandle<String> newStringHandle(ManagedThread::GetCurrent(), newString);
    // After copying we should have a full barrier, so this writes should happen-before barrier
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
    if (compressed) {
        // copy left part
        panda::common::Span<uint8_t> sp(newStringHandle->GetDataUtf8Writable(), newLength);
        panda::common::Span<const uint8_t> src1(str1Handle->GetDataUtf8(), length1);
        panda::BaseString::MemCopyChars(sp, newLength, src1, length1);
        // copy right part
        sp = sp.SubSpan(length1);
        panda::common::Span<const uint8_t> src2(str2Handle->GetDataUtf8(), length2);
        panda::BaseString::MemCopyChars(sp, length2, src2, length2);
    } else {
        // copy left part
        panda::common::Span<uint16_t> sp(newStringHandle->GetDataUtf16Writable(), newLength);
        if (str1Handle->IsUtf8()) {
            panda::BaseString::CopyChars(sp.data(), str1Handle->GetDataUtf8(), length1);
        } else {
            panda::common::Span<const uint16_t> src1(str1Handle->GetDataUtf16(), length1);
            panda::BaseString::MemCopyChars(sp, newLength << 1U, src1, length1 << 1U);
        }
        // copy right part
        sp = sp.SubSpan(length1);
        if (str2Handle->IsUtf8()) {
            panda::BaseString::CopyChars(sp.data(), str2Handle->GetDataUtf8(), length2);
        } else {
            panda::common::Span<const uint16_t> src2(str2Handle->GetDataUtf16(), length2);
            panda::BaseString::MemCopyChars(sp, length2 << 1U, src2, length2 << 1U);
        }
    }
    TSAN_ANNOTATE_IGNORE_WRITES_END();
    // String is supposed to be a constant object, so all its data should be visible by all threads
    arch::FullMemoryBarrier();
    return newStringHandle.GetPtr();
}

/* static */
String *String::GetSlicedString(String *src, uint32_t start, uint32_t length, const LanguageContext &ctx, PandaVM *vm,
                                bool movable, bool pinned)
{
    ASSERT((start + length) <= src->GetLength());
    VMHandle<String> srcHandle(ManagedThread::GetCurrent(), src);

    auto baseStr = String::AllocSlicedStringObject(ctx, vm, movable, pinned);
    if (baseStr == nullptr) {
        return nullptr;
    }
    VMHandle<String> baseStrHandle(ManagedThread::GetCurrent(), baseStr);

    FlatStringInfo srcFlat = FlatStringInfo::FlattenAllString(srcHandle, ctx);
    auto slicedStr = panda::SlicedString::Cast(baseStrHandle->ToString());
    slicedStr->InitLengthAndFlags(length, srcFlat.GetString()->IsMUtf8());
    slicedStr->SetRawHashcode(0);
    auto writeBarrier = [](void *obj, size_t offset, panda::BaseObject *str) {
        ObjectAccessor::SetObject(obj, offset, reinterpret_cast<ObjectHeader *>(str));
    };

    slicedStr->SetParent(std::move(writeBarrier), srcFlat.GetString()->ToString());
    slicedStr->SetStartIndex(start + srcFlat.GetStartIndex());
    return String::Cast(slicedStr);
}

String *String::AllocSlicedStringObject(const LanguageContext &ctx, PandaVM *vm, bool movable, bool pinned)
{
    ASSERT(vm != nullptr);
    auto *thread = ManagedThread::GetCurrent();
    auto *slicedStrCls =
        Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::SLICED_STRING);
    size_t size = panda::SlicedString::SIZE;
    panda::BaseString *slicedStr = nullptr;
    if (movable) {
        slicedStr = reinterpret_cast<panda::BaseString *>(
            vm->GetHeapManager()->AllocateObject(slicedStrCls, size, DEFAULT_ALIGNMENT, thread,
                                                 mem::ObjectAllocatorBase::ObjMemInitPolicy::REQUIRE_INIT, pinned));
    } else {
        slicedStr = reinterpret_cast<panda::BaseString *>(vm->GetHeapManager()->AllocateNonMovableObject(
            slicedStrCls, size, DEFAULT_ALIGNMENT, thread, mem::ObjectAllocatorBase::ObjMemInitPolicy::REQUIRE_INIT));
    }

    slicedStr->SetRawHashcode(0);
    return String::Cast(slicedStr);
}

/**
 * @brief Alloc a TreeString
 * @return The TreeString created
 */
String *String::AllocTreeStringObject(const LanguageContext &ctx, PandaVM *vm, bool movable, bool pinned)
{
    ASSERT(vm != nullptr);
    auto *thread = ManagedThread::GetCurrent();
    auto *treeStrCls = Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::TREE_STRING);
    size_t size = panda::TreeString::SIZE;
    panda::BaseString *treeStr = nullptr;
    if (movable) {
        treeStr = reinterpret_cast<panda::BaseString *>(
            vm->GetHeapManager()->AllocateObject(treeStrCls, size, DEFAULT_ALIGNMENT, thread,
                                                 mem::ObjectAllocatorBase::ObjMemInitPolicy::REQUIRE_INIT, pinned));
    } else {
        treeStr = reinterpret_cast<panda::BaseString *>(vm->GetHeapManager()->AllocateNonMovableObject(
            treeStrCls, size, DEFAULT_ALIGNMENT, thread, mem::ObjectAllocatorBase::ObjMemInitPolicy::REQUIRE_INIT));
    }
    treeStr->SetRawHashcode(0);
    return String::Cast(treeStr);
}

String *String::CreateTreeString(String *left, String *right, uint32_t length, bool compressed,
                                 const LanguageContext &ctx, PandaVM *vm, bool movable, bool pinned)
{
    auto thread = ManagedThread::GetCurrent();
    VMHandle<String> leftHandle(thread, left);
    VMHandle<String> rightHandle(thread, right);

    auto baseStr = String::AllocTreeStringObject(ctx, vm, movable, pinned);
    if (baseStr == nullptr) {
        return nullptr;
    }
    VMHandle<String> baseStrHandle(thread, baseStr);
    auto treeStr = panda::TreeString::Cast(baseStrHandle->ToString());

    // After copying we should have a full barrier, so this writes should happen-before barrier
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();

    treeStr->InitLengthAndFlags(length, compressed);
    treeStr->SetRawHashcode(0);

    auto writeBarrierLeft = [](void *obj, size_t offset, panda::BaseObject *str) {
        ObjectAccessor::SetObject(obj, offset, reinterpret_cast<ObjectHeader *>(str));
    };
    treeStr->SetLeftSubString(std::move(writeBarrierLeft), leftHandle->ToString());
    auto writeBarrierRight = [](void *obj, size_t offset, panda::BaseObject *str) {
        ObjectAccessor::SetObject(obj, offset, reinterpret_cast<ObjectHeader *>(str));
    };
    treeStr->SetRightSubString(std::move(writeBarrierRight), rightHandle->ToString());

    TSAN_ANNOTATE_IGNORE_WRITES_END();
    // String is supposed to be a constant object, so all its data should be visible by all threads
    arch::FullMemoryBarrier();
    return String::Cast(treeStr);
}

int32_t String::IndexOf(String *rhs, const LanguageContext &ctx, int32_t pos)
{
    auto thread = ManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<coretypes::String> receiver(thread, this);
    VMHandle<coretypes::String> search(thread, rhs);
    return String::IndexOf(receiver, search, ctx, pos);
}

int32_t String::LastIndexOf(String *rhs, const LanguageContext &ctx, int32_t pos)
{
    auto thread = ManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<coretypes::String> receiver(thread, this);
    VMHandle<coretypes::String> search(thread, rhs);
    return String::LastIndexOf(receiver, search, ctx, pos);
}

/* static */
int32_t String::IndexOf(VMHandle<String> &receiver, VMHandle<String> &search, const LanguageContext &ctx, int pos)
{
    auto *lhstring = receiver.GetPtr();
    auto *rhstring = search.GetPtr();
    if (lhstring == nullptr || rhstring == nullptr) {
        return -1;
    }
    int32_t lhsCount = static_cast<int32_t>(receiver.GetPtr()->GetLength());  // NOLINT(modernize-use-auto)
    int32_t rhsCount = static_cast<int32_t>(search.GetPtr()->GetLength());    // NOLINT(modernize-use-auto)

    if (pos < 0) {
        pos = 0;
    }

    if (rhsCount == 0) {
        return std::min(lhsCount, pos);
    }

    int32_t max = lhsCount - rhsCount;
    auto thread = ManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    FlatStringInfo lhs = FlatStringInfo::FlattenAllString(receiver, ctx);
    VMHandle<String> string(thread, lhs.GetString());
    FlatStringInfo rhs = FlatStringInfo::FlattenAllString(search, ctx);
    lhs.SetString(string.GetPtr());

    if (rhs.IsUtf8() && lhs.IsUtf8()) {
        panda::common::Span<const uint8_t> lhsSp(lhs.GetDataUtf8(), lhsCount);
        panda::common::Span<const uint8_t> rhsSp(rhs.GetDataUtf8(), rhsCount);
        return panda::BaseString::IndexOf(lhsSp, rhsSp, pos, max);
    } else if (rhs.IsUtf16() && lhs.IsUtf16()) {  // NOLINT(readability-else-after-return)
        panda::common::Span<const uint16_t> lhsSp(lhs.GetDataUtf16(), lhsCount);
        panda::common::Span<const uint16_t> rhsSp(rhs.GetDataUtf16(), rhsCount);
        return panda::BaseString::IndexOf(lhsSp, rhsSp, pos, max);
    } else if (rhs.IsUtf16()) {
        panda::common::Span<const uint8_t> lhsSp(lhs.GetDataUtf8(), lhsCount);
        panda::common::Span<const uint16_t> rhsSp(rhs.GetDataUtf16(), rhsCount);
        return panda::BaseString::IndexOf(lhsSp, rhsSp, pos, max);
    } else {  // NOLINT(readability-else-after-return)
        panda::common::Span<const uint16_t> lhsSp(lhs.GetDataUtf16(), lhsCount);
        panda::common::Span<const uint8_t> rhsSp(rhs.GetDataUtf8(), rhsCount);
        return panda::BaseString::IndexOf(lhsSp, rhsSp, pos, max);
    }
}

/* static */
int32_t String::LastIndexOf(VMHandle<String> &receiver, VMHandle<String> &search, const LanguageContext &ctx, int pos)
{
    auto *lhstring = receiver.GetPtr();
    auto *rhstring = search.GetPtr();
    if (lhstring == nullptr || rhstring == nullptr) {
        return -1;
    }
    int32_t lhsCount = static_cast<int32_t>(receiver.GetPtr()->GetLength());  // NOLINT(modernize-use-auto)
    int32_t rhsCount = static_cast<int32_t>(search.GetPtr()->GetLength());    // NOLINT(modernize-use-auto)

    int32_t max = lhsCount - rhsCount;
    if (pos > max) {
        pos = max;
    }

    if (pos < 0) {
        return -1;
    }

    if (rhsCount == 0) {
        return pos;
    }

    auto thread = ManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    FlatStringInfo lhs = FlatStringInfo::FlattenAllString(receiver, ctx);
    VMHandle<String> string(thread, lhs.GetString());
    FlatStringInfo rhs = FlatStringInfo::FlattenAllString(search, ctx);
    lhs.SetString(string.GetPtr());

    if (rhs.IsUtf8() && lhs.IsUtf8()) {
        panda::common::Span<const uint8_t> lhsSp(lhs.GetDataUtf8(), lhsCount);
        panda::common::Span<const uint8_t> rhsSp(rhs.GetDataUtf8(), rhsCount);
        return panda::BaseString::LastIndexOf(lhsSp, rhsSp, pos);
    } else if (rhs.IsUtf16() && lhs.IsUtf16()) {  // NOLINT(readability-else-after-return)
        panda::common::Span<const uint16_t> lhsSp(lhs.GetDataUtf16(), lhsCount);
        panda::common::Span<const uint16_t> rhsSp(rhs.GetDataUtf16(), rhsCount);
        return panda::BaseString::LastIndexOf(lhsSp, rhsSp, pos);
    } else if (rhs.IsUtf16()) {
        panda::common::Span<const uint8_t> lhsSp(lhs.GetDataUtf8(), lhsCount);
        panda::common::Span<const uint16_t> rhsSp(rhs.GetDataUtf16(), rhsCount);
        return panda::BaseString::LastIndexOf(lhsSp, rhsSp, pos);
    } else {  // NOLINT(readability-else-after-return)
        panda::common::Span<const uint16_t> lhsSp(lhs.GetDataUtf16(), lhsCount);
        panda::common::Span<const uint8_t> rhsSp(rhs.GetDataUtf8(), rhsCount);
        return panda::BaseString::LastIndexOf(lhsSp, rhsSp, pos);
    }
}

/* static */
int32_t String::Compare(VMHandle<String> &left, VMHandle<String> &right, const LanguageContext &ctx)
{
    if (left.GetPtr() == right.GetPtr()) {
        return 0;
    }

    auto thread = ManagedThread::GetCurrent();
    FlatStringInfo lflat = FlatStringInfo::FlattenAllString(left, ctx);
    VMHandle<String> string(thread, lflat.GetString());
    FlatStringInfo rflat = FlatStringInfo::FlattenAllString(right, ctx);
    lflat.SetString(string.GetPtr());

    int32_t lCount = static_cast<int32_t>(lflat.GetLength());  // NOLINT(modernize-use-auto)
    int32_t rCount = static_cast<int32_t>(rflat.GetLength());  // NOLINT(modernize-use-auto)

    int32_t countDiff = lCount - rCount;
    int32_t minCount = (countDiff < 0) ? lCount : rCount;

    if (lflat.IsUtf8()) {
        // left utf8 , right utf8
        if (right.GetPtr()->IsMUtf8()) {
            panda::common::Span<const uint8_t> lspan(lflat.GetDataUtf8(), lCount);
            panda::common::Span<const uint8_t> rspan(rflat.GetDataUtf8(), rCount);
            int32_t charDiff = panda::CompareStringSpan(lspan, rspan, minCount);
            if (charDiff != 0) {
                return charDiff;
            }

            // left utf8 , right utf16
        } else {
            panda::common::Span<const uint8_t> lspan(lflat.GetDataUtf8(), lCount);
            panda::common::Span<const uint16_t> rspan(rflat.GetDataUtf16(), rCount);
            int32_t charDiff = panda::CompareStringSpan(lspan, rspan, minCount);
            if (charDiff != 0) {
                return charDiff;
            }
        }
    } else {
        // left utf16 , right utf16
        if (right.GetPtr()->IsUtf16()) {
            panda::common::Span<const uint16_t> lspan(lflat.GetDataUtf16(), lCount);
            panda::common::Span<const uint16_t> rspan(rflat.GetDataUtf16(), rCount);
            int32_t charDiff = panda::CompareStringSpan(lspan, rspan, minCount);
            if (charDiff != 0) {
                return charDiff;
            }
            // left utf16 , right utf8
        } else {
            panda::common::Span<const uint16_t> lspan(lflat.GetDataUtf16(), lCount);
            panda::common::Span<const uint8_t> rspan(rflat.GetDataUtf8(), rCount);
            int32_t charDiff = panda::CompareStringSpan(lspan, rspan, minCount);
            if (charDiff != 0) {
                return charDiff;
            }
        }
    }
    return countDiff;
}

int32_t String::Compare(String *rstr, const LanguageContext &ctx)
{
    if (this == rstr) {
        return 0;
    }

    auto thread = ManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<String> leftHandle(thread, this);
    VMHandle<String> rightHandle(thread, rstr);

    return Compare(leftHandle, rightHandle, ctx);
}

/* static */
bool String::CanBeCompressedUtf16(const uint16_t *utf16Data, uint32_t utf16Length, uint16_t non)
{
    bool isCompressed = true;
    Span<const uint16_t> data(utf16Data, utf16Length);
    for (uint32_t i = 0; i < utf16Length; i++) {
        if (!panda::BaseString::IsASCIICharacter(data[i]) && data[i] != non) {
            isCompressed = false;
            break;
        }
    }
    return isCompressed;
}

/* static */
bool String::CanBeCompressedMUtf8(const uint8_t *mutf8Data, uint32_t mutf8Length, uint16_t non)
{
    bool isCompressed = true;
    Span<const uint8_t> data(mutf8Data, mutf8Length);
    for (uint32_t i = 0; i < mutf8Length; i++) {
        if (!panda::BaseString::IsASCIICharacter(data[i]) && data[i] != non) {
            isCompressed = false;
            break;
        }
    }
    return isCompressed;
}

String *String::DoReplace(String *src, uint16_t oldC, uint16_t newC, const LanguageContext &ctx, PandaVM *vm)
{
    ASSERT(src != nullptr);
    auto length = static_cast<int32_t>(src->GetLength());
    bool canBeCompressed = panda::BaseString::IsASCIICharacter(newC);

    // allocator may trig gc and move src, need to hold it
    auto thread = ManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<String> srcHandle(thread, src);
    FlatStringInfo srcFlat = FlatStringInfo::FlattenAllString(srcHandle, ctx);
    if (srcFlat.IsUtf16()) {
        canBeCompressed = canBeCompressed && CanBeCompressedUtf16(srcFlat.GetDataUtf16(), length, oldC);
    } else {
        canBeCompressed = canBeCompressed && CanBeCompressedMUtf8(srcFlat.GetDataUtf8(), length, oldC);
    }

    auto string = String::AllocLineStringObject(length, canBeCompressed, ctx, vm);
    if (string == nullptr) {
        return nullptr;
    }

    ASSERT(string->GetHashcode() == 0);

    // After replacing we should have a full barrier, so this writes should happen-before barrier
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
    if (srcFlat.IsUtf16()) {
        if (canBeCompressed) {
            auto replace = [oldC, newC](uint16_t c) { return static_cast<uint8_t>((oldC != c) ? c : newC); };
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            std::transform(srcFlat.GetDataUtf16(), srcFlat.GetDataUtf16() + length, string->GetDataUtf8Writable(),
                           replace);
        } else {
            auto replace = [oldC, newC](uint16_t c) { return (oldC != c) ? c : newC; };
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            std::transform(srcFlat.GetDataUtf16(), srcFlat.GetDataUtf16() + length, string->GetDataUtf16Writable(),
                           replace);
        }
    } else {
        if (canBeCompressed) {
            auto replace = [oldC, newC](uint16_t c) { return static_cast<uint8_t>((oldC != c) ? c : newC); };
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            std::transform(srcFlat.GetDataUtf8(), srcFlat.GetDataUtf8() + length, string->GetDataUtf8Writable(),
                           replace);
        } else {
            auto replace = [oldC, newC](uint16_t c) { return (oldC != c) ? c : newC; };
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            std::transform(srcFlat.GetDataUtf8(), srcFlat.GetDataUtf8() + length, string->GetDataUtf16Writable(),
                           replace);
        }
    }
    TSAN_ANNOTATE_IGNORE_WRITES_END();
    // String is supposed to be a constant object, so all its data should be visible by all threads
    arch::FullMemoryBarrier();
    return string;
}

// static
bool String::CanBeCompressedMUtf8(const uint8_t *mutf8Data)
{
    return utf::IsMUtf8OnlySingleBytes(mutf8Data);
}

bool String::StringsAreEqualMUtf8(String *str1, const uint8_t *mutf8Data, uint32_t utf16Length)
{
    ASSERT(utf16Length == utf::MUtf8ToUtf16Size(mutf8Data));
    if (str1->GetLength() != utf16Length) {
        return false;
    }
    bool canBeCompressed = CanBeCompressedMUtf8(mutf8Data);
    return StringsAreEqualMUtf8(str1, mutf8Data, utf16Length, canBeCompressed);
}

/* static */
bool String::IsMutf8EqualsUtf16(const uint8_t *utf8Data, const uint16_t *utf16Data, uint32_t utf16DataLength)
{
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    auto tmpBuffer = allocator->AllocArray<uint16_t>(utf16DataLength);
    utf::ConvertMUtf8ToUtf16(utf8Data, utf::Mutf8Size(utf8Data), tmpBuffer);
    panda::common::Span<const uint16_t> data1(tmpBuffer, utf16DataLength);
    panda::common::Span<const uint16_t> data2(utf16Data, utf16DataLength);
    bool result = panda::BaseString::StringsAreEquals(data1, data2);
    allocator->Delete(tmpBuffer);
    return result;
}

/* static */
bool String::StringsAreEqualMUtf8(String *str1, const uint8_t *mutf8Data, uint32_t utf16Length, bool canBeCompressed)
{
    bool result = true;
    if (str1->GetLength() != utf16Length) {
        result = false;
    } else {
        bool str1CanBeCompressed = !str1->IsUtf16();
        bool data2CanBeCompressed = canBeCompressed;
        if (str1CanBeCompressed != data2CanBeCompressed) {
            return false;
        }
        ASSERT(str1CanBeCompressed == data2CanBeCompressed);
        auto readBarrier = [](void *obj, size_t offset) {
            return reinterpret_cast<panda::BaseString *>(
                ObjectAccessor::GetObject(const_cast<const void *>(obj), offset));
        };
        if (str1CanBeCompressed) {
            std::vector<uint8_t> bufStr1Utf8;
            int32_t str1Count = static_cast<int32_t>(str1->GetLength());  // NOLINT(modernize-use-auto)
            const uint8_t *str1Utf8DataFlat =
                panda::BaseString::GetUtf8DataFlat(readBarrier, str1->ToString(), bufStr1Utf8);
            panda::common::Span<const uint8_t> data1(str1Utf8DataFlat, str1Count);
            panda::common::Span<const uint8_t> data2(mutf8Data, utf16Length);
            result = panda::BaseString::StringsAreEquals(data1, data2);
        } else {
            std::vector<uint16_t> bufStr1Utf16;
            const uint16_t *str1Utf16DataFlat =
                panda::BaseString::GetUtf16DataFlat(readBarrier, str1->ToString(), bufStr1Utf16);
            result = IsMutf8EqualsUtf16(mutf8Data, str1Utf16DataFlat, str1->GetLength());
        }
    }
    return result;
}

/* static */
bool String::StringsAreEqual(String *str1, String *str2)
{
    auto readBarrier = [](void *obj, size_t offset) {
        return reinterpret_cast<panda::BaseString *>(ObjectAccessor::GetObject(const_cast<const void *>(obj), offset));
    };
    return panda::BaseString::StringsAreEqual(std::move(readBarrier), str1->ToString(), str2->ToString());
}

/* static */
FlatStringInfo FlatStringInfo::FlattenTreeString(VMHandle<String> &treeStr, const LanguageContext &ctx)
{
    panda::TreeString *treeString = treeStr->ToTreeString();
    auto readBarrier = [](void *obj, size_t offset) {
        return reinterpret_cast<panda::BaseString *>(ObjectAccessor::GetObject(const_cast<const void *>(obj), offset));
    };
    if (treeString->IsFlat(std::move(readBarrier))) {
        auto readBarrierLeft = [](void *obj, size_t offset) {
            return reinterpret_cast<panda::BaseString *>(
                ObjectAccessor::GetObject(const_cast<const void *>(obj), offset));
        };
        // NOLINTNEXTLINE(modernize-use-auto)
        panda::BaseString *first = treeString->GetLeftSubString<panda::BaseString *>(std::move(readBarrierLeft));
        return FlatStringInfo(String::Cast(first), 0, treeString->GetLength());
    }

    String *s = SlowFlatten(treeStr, ctx);
    return FlatStringInfo(s, 0, treeString->GetLength());
}

/* static */
FlatStringInfo FlatStringInfo::FlattenSlicedString(VMHandle<String> &slicedStr)
{
    const panda::SlicedString *slicedString = slicedStr->ToSlicedString();
    auto readBarrier = [](void *obj, size_t offset) {
        return reinterpret_cast<panda::BaseString *>(ObjectAccessor::GetObject(const_cast<const void *>(obj), offset));
    };
    // NOLINTNEXTLINE(modernize-use-auto)
    panda::BaseString *parent = slicedString->GetParent<panda::BaseString *>(std::move(readBarrier));
    return FlatStringInfo(String::Cast(parent), slicedString->GetStartIndex(), slicedString->GetLength());
}

/* static */
FlatStringInfo FlatStringInfo::FlattenAllString(VMHandle<String> &str, const LanguageContext &ctx)
{
    String *string = str.GetPtr();
    // 1. LineString return directly
    if (string->IsLineString()) {
        return FlatStringInfo(string, 0, string->GetLength());
    }
    // 2. SlicedString
    if (string->IsSlicedString()) {
        return FlattenSlicedString(str);
    }
    // 3. TreeString
    if (string->IsTreeString()) {
        return FlattenTreeString(str, ctx);
    }
    UNREACHABLE();
    return FlatStringInfo(string, 0, string->GetLength());
}

}  // namespace ark::coretypes
