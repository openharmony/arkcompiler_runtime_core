/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_PLATFORM_TYPES_DEFS_H_
#define PANDA_PLUGINS_ETS_RUNTIME_PLATFORM_TYPES_DEFS_H_

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

// ETS runtime type dependencies list. Each entry MUST follow the common naming schema.
// CC-OFFNXT(G.PRE.06) macro list
#define ETS_PLATFORM_TYPES_LIST(TP, AN, IM, SM)                                                                       \
    /* Core runtime type system */                                                                                    \
    TP("Lstd/core/Object;", coreObject)                                                                               \
    TP("Lstd/core/Class;", coreClass)                                                                                 \
    TP("Lstd/core/Null;", coreNull)                                                                                   \
    TP("Lstd/core/String;", coreString)                                                                               \
    TP("Lstd/core/LineString;", coreLineString)                                                                       \
    TP("Lstd/core/SlicedString;", coreSlicedString)                                                                   \
    TP("Lstd/core/TreeString;", coreTreeString)                                                                       \
    TP("[Lstd/core/String;", coreStringFixedArray)                                                                    \
    /* Builtin numerics */                                                                                            \
    TP("Lstd/core/Boolean;", coreBoolean)                                                                             \
    TP("Lstd/core/Byte;", coreByte)                                                                                   \
    TP("Lstd/core/Char;", coreChar)                                                                                   \
    TP("Lstd/core/Short;", coreShort)                                                                                 \
    TP("Lstd/core/Int;", coreInt)                                                                                     \
    TP("Lstd/core/Long;", coreLong)                                                                                   \
    TP("Lstd/core/Float;", coreFloat)                                                                                 \
    TP("Lstd/core/Double;", coreDouble)                                                                               \
    /* Builtin language support */                                                                                    \
    TP("Lstd/core/BigInt;", coreBigInt)                                                                               \
    TP("Lstd/core/Error;", coreError)                                                                                 \
    TP("Lstd/core/Function;", coreFunction)                                                                           \
    IM("Lstd/core/Function;", "unsafeCall", "[LY;:LY;", coreFunctionUnsafeCall)                                       \
    TP("Lstd/core/Tuple;", coreTuple)                                                                                 \
    TP("Lstd/core/TupleN;", coreTupleN)                                                                               \
    TP("Lstd/core/BaseEnum;", coreBaseEnum)                                                                           \
    /* Builtin language annotations */                                                                                \
    AN("Larkruntime/annotation/InternalAPI;", arkruntimeAnnotationInternalAPI)                                        \
    AN("Larkruntime/annotation/Module;", arkruntimeAnnotationModule)                                                  \
    AN("Larkruntime/annotation/FunctionReference;", arkruntimeAnnotationFunctionReference)                            \
    AN("Larkruntime/annotation/Async;", arkruntimeAnnotationAsync)                                                    \
    AN("Lstd/annotations/InterfaceObjectLiteral;", annotationsInterfaceObjectLiteral)                                 \
    AN("Larkruntime/annotation/OptionalParametersAnnotation;",                                                        \
       arkruntimeAnnotationFunctionsOptionalParametersAnnotation)                                                     \
    AN("Larkruntime/annotation/NamedFunctionObject;", arkruntimeAnnotationNamedFunctionObject)                        \
    AN("Larkruntime/annotation/AsyncFunctionObject;", arkruntimeAnnotationAsyncFunctionObject)                        \
    /* ANI */                                                                                                         \
    AN("Lstd/annotations/ani/unsafe/Quick;", annotationsAniUnsafeQuick)                                               \
    AN("Lstd/annotations/ani/unsafe/Direct;", annotationsAniUnsafeDirect)                                             \
    /* Runtime linkage */                                                                                             \
    TP("Lstd/core/RuntimeLinker;", coreRuntimeLinker)                                                                 \
    IM("Lstd/core/RuntimeLinker;", "loadClass", "Lstd/core/String;Lstd/core/Boolean;:Lstd/core/Class;",               \
       coreRuntimeLinkerLoadClass)                                                                                    \
    TP("Lstd/core/BootRuntimeLinker;", coreBootRuntimeLinker)                                                         \
    TP("Lstd/core/AbcRuntimeLinker;", coreAbcRuntimeLinker)                                                           \
    TP("Lstd/core/MemoryRuntimeLinker;", coreMemoryRuntimeLinker)                                                     \
    TP("Larkruntime/AbcFile;", arkruntimeAbcFile)                                                                     \
    TP("Larkruntime/AbcFileNotFoundError;", arkruntimeAbcFileNotFoundError)                                           \
    /* Linker errors */                                                                                               \
    TP("Lstd/core/LinkerClassNotFoundError;", coreLinkerClassNotFoundError)                                           \
    TP("Lstd/core/LinkerUnresolvedFieldError;", coreLinkerUnresolvedFieldError)                                       \
    TP("Lstd/core/LinkerUnresolvedMethodError;", coreLinkerUnresolvedMethodError)                                     \
    TP("Lstd/core/LinkerUnresolvedClassError;", coreLinkerUnresolvedClassError)                                       \
    TP("Lstd/core/LinkerTypeCircularityError;", coreLinkerTypeCircularityError)                                       \
    TP("Lstd/core/LinkerMethodConflictError;", coreLinkerMethodConflictError)                                         \
    /* Core error handling */                                                                                         \
    TP("Larkruntime/StackTraceElement;", arkruntimeStackTraceElement)                                                 \
    TP("Lstd/core/ErrorOptions;", coreErrorOptions)                                                                   \
    TP("Lstd/core/ErrorOptionsImpl;", coreErrorOptionsImpl)                                                           \
    TP("Lstd/core/OutOfMemoryError;", coreOutOfMemoryError)                                                           \
    TP("Lstd/core/StackOverflowError;", coreStackOverflowError)                                                       \
    TP("Lstd/core/IllegalArgumentError;", coreIllegalArgumentError)                                                   \
    TP("Lstd/core/IllegalStateError;", coreIllegalStateError)                                                         \
    TP("Lstd/core/IllegalLockStateError;", coreIllegalLockStateError)                                                 \
    TP("Lstd/core/UnsupportedOperationError;", coreUnsupportedOperationError)                                         \
    TP("Lstd/core/NegativeArraySizeError;", coreNegativeArraySizeError)                                               \
    TP("Lstd/core/InvalidJobOperationError;", coreInvalidJobOperationError)                                           \
    TP("Lstd/core/RangeError;", coreRangeError)                                                                       \
    TP("Lstd/core/SyntaxError;", coreSyntaxError)                                                                     \
    TP("Lstd/core/IndexOutOfBoundsError;", coreIndexOutOfBoundsError)                                                 \
    TP("Lstd/core/ArrayIndexOutOfBoundsError;", coreArrayIndexOutOfBoundsError)                                       \
    TP("Lstd/core/ArgumentOutOfRangeError;", coreArgumentOutOfRangeError)                                             \
    TP("Lstd/core/ClassCastError;", coreClassCastError)                                                               \
    TP("Lstd/core/ExceptionInInitializerError;", coreExceptionInInitializerError)                                     \
    TP("Lstd/core/NullPointerError;", coreNullPointerError)                                                           \
    TP("Lstd/core/TypeError;", coreTypeError)                                                                         \
    TP("Lstd/core/FormatError;", coreFormatError)                                                                     \
    TP("Lstd/core/ReferenceError;", coreReferenceError)                                                               \
    TP("Lstd/core/URIError;", coreURIError)                                                                           \
    /* ToStringCache */                                                                                               \
    TP("Lstd/core/DoubleToStringCacheElement;", coreDoubleToStringCacheElement)                                       \
    TP("Lstd/core/FloatToStringCacheElement;", coreFloatToStringCacheElement)                                         \
    TP("Lstd/core/LongToStringCacheElement;", coreLongToStringCacheElement)                                           \
    /* StringBuilder */                                                                                               \
    TP("Lstd/core/StringBuilder;", coreStringBuilder)                                                                 \
    IM("Lstd/core/StringBuilder;", "<ctor>", ":V", coreStringBuilderDefaultConstructor)                               \
    IM("Lstd/core/StringBuilder;", "<ctor>", "[C:V", coreStringBuilderConstructorWithCharArrayArg)                    \
    IM("Lstd/core/StringBuilder;", "<ctor>", "Lstd/core/String;:V", coreStringBuilderConstructorWithStringArg)        \
    IM("Lstd/core/StringBuilder;", "%%get-stringLength", ":I", coreStringBuilderStringLength)                         \
    IM("Lstd/core/StringBuilder;", "append", "Lstd/core/String;:Lstd/core/StringBuilder;",                            \
       coreStringBuilderAppendString)                                                                                 \
    IM("Lstd/core/StringBuilder;", "toString", ":Lstd/core/String;", coreStringBuilderToString)                       \
    /* String */                                                                                                      \
    IM("Lstd/core/String;", "concat", "Lstd/core/Array;:Lstd/core/String;", coreStringConcat)                         \
    IM("Lstd/core/String;", "getLength", ":I", coreStringGetLength)                                                   \
    IM("Lstd/core/String;", "%%get-length", ":I", coreStringLength)                                                   \
    /* Object array */                                                                                                \
    TP("[Lstd/core/Object;", coreObjectArray)                                                                         \
    /* Concurrency */                                                                                                 \
    TP("Larkruntime/AsyncContext;", arkruntimeAsyncContext)                                                           \
    TP("Lstd/core/Promise;", corePromise)                                                                             \
    TP("Lstd/core/Job;", coreJob)                                                                                     \
    IM("Lstd/core/Promise;", "subscribeOnAnotherPromise", "Lstd/core/PromiseLike;:V",                                 \
       corePromiseSubscribeOnAnotherPromise)                                                                          \
    TP("Lstd/core/PromiseRef;", corePromiseRef)                                                                       \
    TP("Lstd/core/WaitersList;", coreWaitersList)                                                                     \
    TP("Lstd/core/Mutex;", coreMutex)                                                                                 \
    TP("Lstd/core/Event;", coreEvent)                                                                                 \
    TP("Lstd/core/CondVar;", coreCondVar)                                                                             \
    TP("Lstd/core/QueueSpinlock;", coreQueueSpinlock)                                                                 \
    TP("Lstd/core/RWLock;", coreRWLock)                                                                               \
    /* Taskpool */                                                                                                    \
    TP("Lstd/concurrency/taskpool;", stdConcurrencyTaskpool)                                                          \
    SM("Lstd/concurrency/taskpool;", "setTaskPoolBlockedWorkerThreshold", "I:V",                                      \
       stdConcurrencyTaskpoolSetTaskPoolBlockedWorkerThreshold)                                                       \
    SM("Lstd/concurrency/taskpool;", "setTaskPoolBlockedWorkerMonitorInterval", "I:V",                                \
       stdConcurrencyTaskpoolSetTaskPoolBlockedWorkerMonitorInterval)                                                 \
    SM("Lstd/concurrency/taskpool;", "retriggerTaskPoolBlockedExpandMonitor", ":V",                                   \
       stdConcurrencyTaskpoolRetriggerTaskPoolBlockedExpandMonitor)                                                   \
    TP("Lstd/core/EAWorker;", coreEAWorker)                                                                           \
    SM("Lstd/core/EAWorker;", "handleInteropEnvError", ":V", coreEAWorkerHandleInteropEnvError)                       \
    /* Finalization */                                                                                                \
    TP("Lstd/core/BaseWeakRef;", coreBaseWeakRef)                                                                     \
    TP("Lstd/core/FinalizableWeakRef;", coreFinalizableWeakRef)                                                       \
    TP("Lstd/core/FinalizationRegistry;", coreFinalizationRegistry)                                                   \
    SM("Lstd/core/FinalizationRegistry;", "execCleanup", "Lstd/core/FinRegNode;:V",                                   \
       coreFinalizationRegistryExecCleanup)                                                                           \
    TP("Lstd/core/FinRegNode;", coreFinRegNode)                                                                       \
    /* Containers */                                                                                                  \
    TP("Lstd/core/Array;", escompatArray)                                                                             \
    IM("Lstd/core/Array;", "pop", ":LY;", escompatArrayPop)                                                           \
    IM("Lstd/core/Array;", "%%get-length", ":I", escompatArrayGetLength)                                              \
    IM("Lstd/core/Array;", "$_get", "I:LY;", escompatArrayGet)                                                        \
    IM("Lstd/core/Array;", "$_set", "ILY;:V", escompatArraySet)                                                       \
    /* ArrayBuffer */                                                                                                 \
    TP("Lstd/core/ArrayBuffer;", coreArrayBuffer)                                                                     \
    TP("Lstd/core/DataView;", coreDataView)                                                                           \
    TP("Lstd/core/Int8Array;", coreInt8Array)                                                                         \
    TP("Lstd/core/Uint8Array;", coreUint8Array)                                                                       \
    TP("Lstd/core/Uint8ClampedArray;", coreUint8ClampedArray)                                                         \
    TP("Lstd/core/Int16Array;", coreInt16Array)                                                                       \
    TP("Lstd/core/Uint16Array;", coreUint16Array)                                                                     \
    TP("Lstd/core/Int32Array;", coreInt32Array)                                                                       \
    TP("Lstd/core/Uint32Array;", coreUint32Array)                                                                     \
    TP("Lstd/core/Float32Array;", coreFloat32Array)                                                                   \
    TP("Lstd/core/Float64Array;", coreFloat64Array)                                                                   \
    TP("Lstd/core/BigInt64Array;", coreBigInt64Array)                                                                 \
    TP("Lstd/core/BigUint64Array;", coreBigUint64Array)                                                               \
    /* Containers */                                                                                                  \
    TP("Lstd/containers/containers/ArrayAsListInt;", containersArrayAsListInt)                                        \
    TP("Lstd/core/ArrayLike;", coreArrayLike)                                                                         \
    TP("Lstd/core/Record;", coreRecord)                                                                               \
    IM("Lstd/core/Record;", "$_get", "{ULstd/core/BaseEnum;Lstd/core/Numeric;Lstd/core/String;}:LY;", coreRecordGet)  \
    IM("Lstd/core/Record;", "$_set", "{ULstd/core/BaseEnum;Lstd/core/Numeric;Lstd/core/String;}LY;:V", coreRecordSet) \
    TP("Lstd/core/Map;", coreMap)                                                                                     \
    TP("Lstd/core/Set;", coreSet)                                                                                     \
    /* Iterators */                                                                                                   \
    TP("Lstd/core/MapIteratorImpl;", coreMapIteratorImpl)                                                             \
    TP("Lstd/core/SetIteratorImpl;", coreSetIteratorImpl)                                                             \
    TP("Lstd/core/EmptyMapIteratorImpl;", coreEmptyMapIteratorImpl)                                                   \
    TP("Lstd/core/ArrayEntriesIterator_T;", escompatArrayEntriesIteratorT)                                            \
    TP("Lstd/core/ArrayKeysIterator;", coreArrayKeysIterator)                                                         \
    TP("Lstd/core/ArrayValuesIterator_T;", coreArrayValuesIteratorT)                                                  \
    TP("Lstd/core/IteratorResult;", coreIteratorResult)                                                               \
    /* Interop */                                                                                                     \
    TP("Lstd/interop/js/JSRuntime;", interopJSRuntime)                                                                \
    TP("Lstd/interop/js/JSValue;", interopJSValue)                                                                    \
    TP("Lstd/interop/ESValue;", interopESValue)                                                                       \
    TP("Lstd/interop/js/ESError;", interopESError)                                                                    \
    TP("Lstd/interop/js/NoInteropContextError;", interopNoInteropContextError)                                        \
    TP("Lstd/interop/js/DynamicFunction;", interopDynamicFunction)                                                    \
    TP("Lstd/interop/js/PromiseInterop;", interopPromiseInterop)                                                      \
    SM("Lstd/interop/js/PromiseInterop;", "connectPromise", "Lstd/core/Promise;J:V",                                  \
       interopPromiseInteropConnectPromise)                                                                           \
    /* TypeAPI */                                                                                                     \
    TP("Lstd/core/Field;", coreField)                                                                                 \
    TP("Lstd/core/Method;", coreMethod)                                                                               \
    TP("Lstd/core/TypeAPIParameter;", coreTypeAPIParameter)                                                           \
    TP("Lstd/core/ClassType;", coreClassType)                                                                         \
    TP("Lstd/core/reflect/InstanceField;", coreReflectInstanceField)                                                  \
    TP("Lstd/core/reflect/InstanceMethod;", coreReflectInstanceMethod)                                                \
    TP("Lstd/core/reflect/StaticField;", coreReflectStaticField)                                                      \
    TP("Lstd/core/reflect/StaticMethod;", coreReflectStaticMethod)                                                    \
    TP("Lstd/core/reflect/Constructor;", coreReflectConstructor)                                                      \
    /* Proxy */                                                                                                       \
    TP("Lstd/core/reflect/Proxy;", coreReflectProxy)                                                                  \
    IM("Lstd/core/reflect/Proxy;", "<ctor>", "Lstd/core/reflect/InvocationHandler;:V", coreReflectProxyConstructor)   \
    IM("Lstd/core/reflect/Proxy;", "getHandler", ":Lstd/core/reflect/InvocationHandler;", coreReflectProxyGetHandler) \
    SM("Lstd/core/reflect/Proxy;", "invoke", "Lstd/core/reflect/Proxy;Lstd/core/reflect/InstanceMethod;[LY;:LY;",     \
       coreReflectProxyInvoke)                                                                                        \
    SM("Lstd/core/reflect/Proxy;", "invokeSet", "Lstd/core/reflect/Proxy;Lstd/core/reflect/InstanceMethod;LY;:V",     \
       coreReflectProxyInvokeSet)                                                                                     \
    SM("Lstd/core/reflect/Proxy;", "invokeGet", "Lstd/core/reflect/Proxy;Lstd/core/reflect/InstanceMethod;:LY;",      \
       coreReflectProxyInvokeGet)                                                                                     \
    /* Process */                                                                                                     \
    TP("Lstd/core/StdProcess;", coreStdProcess)                                                                       \
    SM("Lstd/core/StdProcess;", "listUnhandledJobs", "Lstd/core/Array;I:V", coreStdProcessListUnhandledJobs)          \
    SM("Lstd/core/StdProcess;", "listUnhandledPromises", "Lstd/core/Array;I:V", coreStdProcessListUnhandledPromises)  \
    SM("Lstd/core/StdProcess;", "HandleUncaughtError", "Lstd/core/Object;:V", coreStdProcessHandleUncaughtError)      \
    /* JSON */                                                                                                        \
    TP("Lstd/core/JSON;", coreJSON)                                                                                   \
    SM("Lstd/core/JSON;", "stringify", "LY;:Lstd/core/String;", coreJSONStringify)                                    \
    TP("Lstd/core/JsonReplacer;", coreJsonReplacer)                                                                   \
    TP("Lstd/core/jsonx/JsonElementSerializable;", coreJsonElementSerializable)                                       \
    TP("Lstd/core/JsonSerializable;", coreJsonSerializable)                                                           \
    IM("Lstd/core/JsonSerializable;", "toJSON", ":Lstd/core/String;", coreJsonSerializableToJSON)                     \
    AN("Lstd/core/JSONStringifyIgnore;", coreJSONStringifyIgnore)                                                     \
    AN("Lstd/core/JSONParseIgnore;", coreJSONParseIgnore)                                                             \
    AN("Lstd/core/JSONRename;", coreJSONRename)                                                                       \
    AN("Lstd/core/JSONStringifyGetter;", coreJSONStringifyGetter)                                                     \
    /* RegExp */                                                                                                      \
    TP("Lstd/core/RegExpResultArray;", coreRegExpResultArray)                                                         \
    TP("Lstd/core/RegExp;", coreRegExp)                                                                               \
    /* Date */                                                                                                        \
    TP("Lstd/core/Date;", coreDate)                                                                                   \
    /* Do not add anything below this line. */

// NOLINTEND(cppcoreguidelines-macro-usage)

#endif  // PANDA_PLUGINS_ETS_RUNTIME_PLATFORM_TYPES_DEFS_H_
