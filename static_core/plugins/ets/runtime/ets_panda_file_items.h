/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
static constexpr std::string_view OBJECT                               = "Lstd/core/Object;";
static constexpr std::string_view PROMISE                              = "Lstd/core/Promise;";
static constexpr std::string_view JOB                                  = "Lstd/core/Job;";
static constexpr std::string_view PROMISE_REF                          = "Lstd/core/PromiseRef;";
static constexpr std::string_view WAITERS_LIST                         = "Lstd/core/WaitersList;";
static constexpr std::string_view MUTEX                                = "Lstd/core/Mutex;";
static constexpr std::string_view EVENT                                = "Lstd/core/Event;";
static constexpr std::string_view COND_VAR                             = "Lstd/core/CondVar;";
static constexpr std::string_view QUEUE_SPINLOCK                       = "Lstd/core/QueueSpinlock;";
static constexpr std::string_view NULL_VALUE                           = "Lstd/core/__NullValue;";
static constexpr std::string_view STRING                               = "Lstd/core/String;";
static constexpr std::string_view WEAK_REF                             = "Lstd/core/WeakRef;";
static constexpr std::string_view FINALIZABLE_WEAK_REF                 = "Lstd/core/FinalizableWeakRef;";
static constexpr std::string_view FINALIZATION_REGISTRY                = "Lstd/core/FinalizationRegistry;";
static constexpr std::string_view TYPE                                 = "Lstd/core/Type;";
static constexpr std::string_view FIELD                                = "Lstd/core/Field;";
static constexpr std::string_view METHOD                               = "Lstd/core/Method;";
static constexpr std::string_view PARAMETER                            = "Lstd/core/Parameter;";
static constexpr std::string_view STRING_BUILDER                       = "Lstd/core/StringBuilder;";

// Runtime classes
static constexpr std::string_view STACK_TRACE_ELEMENT                  = "Lstd/core/StackTraceElement;";
static constexpr std::string_view ABC_FILE                             = "Lstd/core/AbcFile;";
static constexpr std::string_view ABC_RUNTIME_LINKER                   = "Lstd/core/AbcRuntimeLinker;";
static constexpr std::string_view BOOT_RUNTIME_LINKER                  = "Lstd/core/BootRuntimeLinker;";
static constexpr std::string_view CLASS                                = "Lstd/core/Class;";
static constexpr std::string_view RUNTIME_LINKER                       = "Lstd/core/RuntimeLinker;";

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
static constexpr std::string_view FUNCTION                             = "Lstd/core/Function;";
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

// Tuple classes
static constexpr std::string_view TUPLE                                = "Lstd/core/Tuple;";
static constexpr std::string_view TUPLE0                               = "Lstd/core/Tuple0;";
static constexpr std::string_view TUPLE1                               = "Lstd/core/Tuple1;";
static constexpr std::string_view TUPLE2                               = "Lstd/core/Tuple2;";
static constexpr std::string_view TUPLE3                               = "Lstd/core/Tuple3;";
static constexpr std::string_view TUPLE4                               = "Lstd/core/Tuple4;";
static constexpr std::string_view TUPLE5                               = "Lstd/core/Tuple5;";
static constexpr std::string_view TUPLE6                               = "Lstd/core/Tuple6;";
static constexpr std::string_view TUPLE7                               = "Lstd/core/Tuple7;";
static constexpr std::string_view TUPLE8                               = "Lstd/core/Tuple8;";
static constexpr std::string_view TUPLE9                               = "Lstd/core/Tuple9;";
static constexpr std::string_view TUPLE10                              = "Lstd/core/Tuple10;";
static constexpr std::string_view TUPLE11                              = "Lstd/core/Tuple11;";
static constexpr std::string_view TUPLE12                              = "Lstd/core/Tuple12;";
static constexpr std::string_view TUPLE13                              = "Lstd/core/Tuple13;";
static constexpr std::string_view TUPLE14                              = "Lstd/core/Tuple14;";
static constexpr std::string_view TUPLE15                              = "Lstd/core/Tuple15;";
static constexpr std::string_view TUPLE16                              = "Lstd/core/Tuple16;";
static constexpr std::string_view TUPLEN                               = "Lstd/core/TupleN;";

// Base type for all enums
static constexpr std::string_view BASE_ENUM                            = "Lstd/core/BaseEnum;";

// core-defined error classes
static constexpr std::string_view ABC_FILE_NOT_FOUND_ERROR             = "Lstd/core/AbcFileNotFoundError;";
static constexpr std::string_view ARITHMETIC_ERROR                     = "Lstd/core/ArithmeticError;";
static constexpr std::string_view ARRAY_INDEX_OUT_OF_BOUNDS_ERROR      = "Lstd/core/ArrayIndexOutOfBoundsError;";
static constexpr std::string_view ARRAY_STORE_ERROR                    = "Lstd/core/ArrayStoreError;";
static constexpr std::string_view CLASS_CAST_ERROR                     = "Lstd/core/ClassCastError;";
static constexpr std::string_view COROUTINES_LIMIT_EXCEED_ERROR        = "Lstd/core/CoroutinesLimitExceedError;";
static constexpr std::string_view EXCEPTION_IN_INITIALIZER_ERROR       = "Lstd/core/ExceptionInInitializerError;";
static constexpr std::string_view FILE_NOT_FOUND_ERROR                 = "Lstd/core/FileNotFoundError;";
static constexpr std::string_view ILLEGAL_ACCESS_ERROR                 = "Lstd/core/IllegalAccessError;";
// remove or make an Error
static constexpr std::string_view ILLEGAL_ARGUMENT_EXCEPTION           = "Lstd/core/IllegalArgumentException;";
static constexpr std::string_view ILLEGAL_MONITOR_STATE_ERROR          = "Lstd/core/IllegalMonitorStateError;";
// remove or make an Error
static constexpr std::string_view ILLEGAL_STATE_EXCEPTION              = "Lstd/core/IllegalStateException;";
static constexpr std::string_view INDEX_OUT_OF_BOUNDS_ERROR            = "Lstd/core/IndexOutOfBoundsError;";
static constexpr std::string_view INSTANTIATION_ERROR                  = "Lstd/core/InstantiationError;";
// has no class defined
static constexpr std::string_view IO_ERROR                             = "Lstd/core/IOError;";
static constexpr std::string_view NEGATIVE_ARRAY_SIZE_ERROR            = "Lstd/core/NegativeArraySizeError;";
static constexpr std::string_view LINKER_ABSTRACT_METHOD_ERROR         = "Lstd/core/LinkerAbstractMethodError;";
static constexpr std::string_view LINKER_TYPE_CIRCULARITY_ERROR        = "Lstd/core/LinkerTypeCircularityError;";
static constexpr std::string_view LINKER_CLASS_NOT_FOUND_ERROR         = "Lstd/core/LinkerClassNotFoundError;";
static constexpr std::string_view LINKER_BAD_SUPERTYPE_ERROR           = "Lstd/core/LinkerBadSupertypeError;";
static constexpr std::string_view LINKER_UNRESOLVED_CLASS_ERROR        = "Lstd/core/LinkerUnresolvedClassError;";
static constexpr std::string_view LINKER_UNRESOLVED_FIELD_ERROR        = "Lstd/core/LinkerUnresolvedFieldError;";
static constexpr std::string_view LINKER_UNRESOLVED_METHOD_ERROR       = "Lstd/core/LinkerUnresolvedMethodError;";
static constexpr std::string_view LINKER_METHOD_CONFLICT_ERROR         = "Lstd/core/LinkerMethodConflictError;";
static constexpr std::string_view LINKER_VERIFICATION_ERROR            = "Lstd/core/LinkerVerificationError;";
static constexpr std::string_view NULL_POINTER_ERROR                   = "Lstd/core/NullPointerError;";
static constexpr std::string_view OUT_OF_MEMORY_ERROR                  = "Lstd/core/OutOfMemoryError;";
static constexpr std::string_view RANGE_ERROR                          = "Lstd/core/RangeError;";
static constexpr std::string_view TYPE_ERROR                           = "Lescompat/TypeError;";
// remove or make an Error
static constexpr std::string_view RUNTIME_EXCEPTION                    = "Lstd/core/RuntimeException;";
static constexpr std::string_view STACK_OVERFLOW_ERROR                 = "Lstd/core/StackOverflowError;";
static constexpr std::string_view STRING_INDEX_OUT_OF_BOUNDS_ERROR     = "Lstd/core/StringIndexOutOfBoundsError;";
// remove or make an Error
static constexpr std::string_view UNSUPPORTED_OPERATION_EXCEPTION      = "Lstd/core/UnsupportedOperationException;";

// coroutines
static constexpr std::string_view INVALID_COROUTINE_OPERATION_ERROR    = "Lstd/core/InvalidCoroutineOperationError;";

// stdlib Exception classes
static constexpr std::string_view ARGUMENT_OUT_OF_RANGE_EXCEPTION      = "Lstd/core/ArgumentOutOfRangeException;";

// stdlib Error classes
static constexpr std::string_view ERROR_OPTIONS                        = "Lescompat/ErrorOptions;";

static constexpr std::string_view DOUBLE_TO_STRING_CACHE_ELEMENT       = "Lstd/core/DoubleToStringCacheElement;";
static constexpr std::string_view FLOAT_TO_STRING_CACHE_ELEMENT        = "Lstd/core/FloatToStringCacheElement;";
static constexpr std::string_view LONG_TO_STRING_CACHE_ELEMENT         = "Lstd/core/LongToStringCacheElement;";

// interop
static constexpr std::string_view NO_INTEROP_CONTEXT_ERROR             = "Lstd/interop/js/NoInteropContextError;";

// interop/js
static constexpr std::string_view JS_RUNTIME                           = "Lstd/interop/js/JSRuntime;";
static constexpr std::string_view JS_VALUE                             = "Lstd/interop/js/JSValue;";
static constexpr std::string_view ES_ERROR                             = "Lstd/interop/js/ESError;";

static constexpr std::string_view ARRAY                                = "Lescompat/Array;";
static constexpr std::string_view ARRAY_AS_LIST_INT                    = "Lstd/containers/ArrayAsListInt;";

// ANI annotation classes
static constexpr std::string_view ANI_UNSAFE_QUICK                     = "Lstd/annotations/ani/unsafe/Quick;";
static constexpr std::string_view ANI_UNSAFE_DIRECT                    = "Lstd/annotations/ani/unsafe/Direct;";

// Module annotation class
static constexpr std::string_view ANNOTATION_MODULE                    = "Lets/annotation/Module;";

// escompat
static constexpr std::string_view SHARED_MEMORY                        = "Lescompat/SharedMemory;";
static constexpr std::string_view DATE                                 = "Lescompat/Date;";
static constexpr std::string_view ARRAY_ENTRIES_ITERATOR_T             = "Lescompat/ArrayEntriesIterator_T;";
static constexpr std::string_view ITERATOR_RESULT                      = "Lescompat/IteratorResult;";
static constexpr std::string_view ARRAY_KEYS_ITERATOR                  = "Lescompat/ArrayKeysIterator;";
static constexpr std::string_view ARRAY_VALUES_ITERATOR_T              = "Lescompat/ArrayValuesIterator_T;";

}  // namespace class_descriptors

static constexpr std::string_view CCTOR = "<cctor>";
static constexpr std::string_view CTOR  = "<ctor>";
// clang-format on

}  // namespace ark::ets::panda_file_items

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ETS_PANDA_FILE_ITEMS_H_
