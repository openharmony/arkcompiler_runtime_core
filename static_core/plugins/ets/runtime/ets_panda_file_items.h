/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_PANDA_FILE_ITEMS_H_
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_PANDA_FILE_ITEMS_H_

#include <string_view>

namespace ark::ets::panda_file_items {

// clang-format off
namespace class_descriptors {

// Base classes
static constexpr std::string_view ERROR                                = "Lescompat/Error;";
static constexpr std::string_view ARRAY_BUFFER                         = "Lescompat/ArrayBuffer;";
static constexpr std::string_view BIG_INT                              = "Lescompat/BigInt;";
static constexpr std::string_view ASYNC                                = "Lets/coroutine/Async;";
static constexpr std::string_view EXCEPTION                            = "Lstd/core/Exception;";
static constexpr std::string_view CLASS                                = "Lstd/core/Class;";
static constexpr std::string_view OBJECT                               = "Lstd/core/Object;";
static constexpr std::string_view PROMISE                              = "Lstd/core/Promise;";
static constexpr std::string_view PROMISE_REF                          = "Lstd/core/PromiseRef;";
static constexpr std::string_view WAITERS_LIST                         = "Lstd/core/WaitersList;";
static constexpr std::string_view MUTEX                                = "Lstd/core/Mutex;";
static constexpr std::string_view EVENT                                = "Lstd/core/Event;";
static constexpr std::string_view COND_VAR                             = "Lstd/core/CondVar;";
static constexpr std::string_view INTERNAL_UNDEFINED                   = "Lstd/core/__internal_undefined;";
static constexpr std::string_view STRING                               = "Lstd/core/String;";
static constexpr std::string_view WEAK_REF                             = "Lstd/core/WeakRef;";
static constexpr std::string_view FINALIZABLE_WEAK_REF                 = "Lstd/core/FinalizableWeakRef;";
static constexpr std::string_view TYPE                                 = "Lstd/core/Type;";
static constexpr std::string_view FIELD                                = "Lstd/core/Field;";
static constexpr std::string_view METHOD                               = "Lstd/core/Method;";
static constexpr std::string_view PARAMETER                            = "Lstd/core/Parameter;";
static constexpr std::string_view STRING_BUILDER                       = "Lstd/core/StringBuilder;";

// Runtime classes
static constexpr std::string_view STACK_TRACE_ELEMENT                  = "Lstd/core/StackTraceElement;";

// Box classes
static constexpr std::string_view BOX_BOOLEAN                          = "Lstd/core/Boolean;";
static constexpr std::string_view BOX_BYTE                             = "Lstd/core/Byte;";
static constexpr std::string_view BOX_CHAR                             = "Lstd/core/Char;";
static constexpr std::string_view BOX_SHORT                            = "Lstd/core/Short;";
static constexpr std::string_view BOX_INT                              = "Lstd/core/Int;";
static constexpr std::string_view BOX_LONG                             = "Lstd/core/Long;";
static constexpr std::string_view BOX_FLOAT                            = "Lstd/core/Float;";
static constexpr std::string_view BOX_DOUBLE                           = "Lstd/core/Double;";

// Arrays of base classes
static constexpr std::string_view CLASS_ARRAY                          = "[Lstd/core/Class;";
static constexpr std::string_view STRING_ARRAY                         = "[Lstd/core/String;";

// Functional interfaces
static constexpr std::string_view FUNCTION0                            = "Lstd/core/Function0;";
static constexpr std::string_view FUNCTION1                            = "Lstd/core/Function1;";
static constexpr std::string_view FUNCTION2                            = "Lstd/core/Function2;";
static constexpr std::string_view FUNCTION3                            = "Lstd/core/Function3;";
static constexpr std::string_view FUNCTION4                            = "Lstd/core/Function4;";
static constexpr std::string_view FUNCTION5                            = "Lstd/core/Function5;";
static constexpr std::string_view FUNCTION6                            = "Lstd/core/Function6;";
static constexpr std::string_view FUNCTION7                            = "Lstd/core/Function7;";
static constexpr std::string_view FUNCTION8                            = "Lstd/core/Function8;";
static constexpr std::string_view FUNCTION9                            = "Lstd/core/Function9;";
static constexpr std::string_view FUNCTION10                           = "Lstd/core/Function10;";
static constexpr std::string_view FUNCTION11                           = "Lstd/core/Function11;";
static constexpr std::string_view FUNCTION12                           = "Lstd/core/Function12;";
static constexpr std::string_view FUNCTION13                           = "Lstd/core/Function13;";
static constexpr std::string_view FUNCTION14                           = "Lstd/core/Function14;";
static constexpr std::string_view FUNCTION15                           = "Lstd/core/Function15;";
static constexpr std::string_view FUNCTION16                           = "Lstd/core/Function16;";
static constexpr std::string_view FUNCTIONN                            = "Lstd/core/FunctionN;";
static constexpr std::string_view IFUNCTION                    = "Lstd/core/Function;";

// Exception classes
static constexpr std::string_view ARGUMENT_OUT_OF_RANGE_EXCEPTION      = "Lstd/core/ArgumentOutOfRangeException;";
static constexpr std::string_view CLASS_NOT_FOUND_EXCEPTION            = "Lstd/core/ClassNotFoundException;";
static constexpr std::string_view FILE_NOT_FOUND_EXCEPTION             = "Lstd/core/FileNotFoundException;";
static constexpr std::string_view ILLEGAL_ACCESS_EXCEPTION             = "Lstd/core/IllegalAccessException;";
static constexpr std::string_view ILLEGAL_ARGUMENT_EXCEPTION           = "Lstd/core/IllegalArgumentException;";
static constexpr std::string_view ILLEGAL_MONITOR_STATE_EXCEPTION      = "Lstd/core/IllegalMonitorStateException;";
static constexpr std::string_view ILLEGAL_STATE_EXCEPTION              = "Lstd/core/IllegalStateException;";
static constexpr std::string_view IO_EXCEPTION                         = "Lstd/core/IOException;";
static constexpr std::string_view RUNTIME_EXCEPTION                    = "Lstd/core/RuntimeException;";
static constexpr std::string_view UNSUPPORTED_OPERATION_EXCEPTION      = "Lstd/core/UnsupportedOperationException;";

// Exceptions classes
// Due to dependecy of runtime architecture, have confusing name
static constexpr std::string_view ABSTRACT_METHOD_ERROR                = "Lstd/core/AbstractMethodError;";
static constexpr std::string_view ARITHMETIC_ERROR                     = "Lstd/core/ArithmeticError;";
static constexpr std::string_view ARRAY_STORE_ERROR                    = "Lstd/core/ArrayStoreError;";
static constexpr std::string_view CLASS_CAST_ERROR                     = "Lstd/core/ClassCastError;";
static constexpr std::string_view ARRAY_INDEX_OUT_OF_BOUNDS_ERROR      = "Lstd/core/ArrayIndexOutOfBoundsError;";
static constexpr std::string_view CLASS_CIRCULARITY_ERROR              = "Lstd/core/ClassCircularityError;";
static constexpr std::string_view EXCEPTION_IN_INITIALIZER_ERROR       = "Lstd/core/ExceptionInInitializerError;";
static constexpr std::string_view INCOMPATIBLE_CLASS_CHANGE_ERROR      = "Lstd/core/IncompatibleClassChangeError;";
static constexpr std::string_view INDEX_OUT_OF_BOUNDS_ERROR            = "Lstd/core/IndexOutOfBoundsError;";
static constexpr std::string_view INSTANTIATION_ERROR                  = "Lstd/core/InstantiationError;";
static constexpr std::string_view LINKAGE_ERROR                        = "Lstd/core/LinkageError;";
static constexpr std::string_view NEGATIVE_ARRAY_SIZE_ERROR            = "Lstd/core/NegativeArraySizeError;";
static constexpr std::string_view NO_CLASS_DEF_FOUND_ERROR             = "Lstd/core/NoClassDefFoundError;";
static constexpr std::string_view NO_SUCH_FIELD_ERROR                  = "Lstd/core/NoSuchFieldError;";
static constexpr std::string_view NO_SUCH_METHOD_ERROR                 = "Lstd/core/NoSuchMethodError;";
static constexpr std::string_view NULL_POINTER_ERROR                   = "Lstd/core/NullPointerError;";
static constexpr std::string_view OUT_OF_MEMORY_ERROR                  = "Lstd/core/OutOfMemoryError;";
static constexpr std::string_view STACK_OVERFLOW_ERROR                 = "Lstd/core/StackOverflowError;";
static constexpr std::string_view STRING_INDEX_OUT_OF_BOUNDS_ERROR     = "Lstd/core/StringIndexOutOfBoundsError;";
static constexpr std::string_view VERIFY_ERROR                         = "Lstd/core/VerifyError;";

static constexpr std::string_view DOUBLE_TO_STRING_CACHE_ELEMENT       = "Lstd/core/DoubleToStringCacheElement;";
static constexpr std::string_view FLOAT_TO_STRING_CACHE_ELEMENT        = "Lstd/core/FloatToStringCacheElement;";
static constexpr std::string_view LONG_TO_STRING_CACHE_ELEMENT         = "Lstd/core/LongToStringCacheElement;";

// Error classes
static constexpr std::string_view ERROR_OPTIONS                        = "Lescompat/ErrorOptions;";
static constexpr std::string_view RANGE_ERROR                          = "Lescompat/RangeError;";

// interop/js
static constexpr std::string_view JS_RUNTIME                           = "Lstd/interop/js/JSRuntime;";
static constexpr std::string_view JS_VALUE                             = "Lstd/interop/js/JSValue;";
static constexpr std::string_view JS_ERROR                             = "Lstd/interop/js/JSError;";

static constexpr std::string_view ARRAY                                = "Lescompat/Array;";
static constexpr std::string_view ARRAY_AS_LIST_INT                    = "Lstd/containers/ArrayAsListInt;";

// escompat
static constexpr std::string_view SHARED_MEMORY                        = "Lescompat/SharedMemory;";

}  // namespace class_descriptors

static constexpr std::string_view CCTOR = "<cctor>";
static constexpr std::string_view CTOR  = "<ctor>";
// clang-format on

}  // namespace ark::ets::panda_file_items

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ETS_PANDA_FILE_ITEMS_H_
