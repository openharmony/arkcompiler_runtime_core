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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_INTEROP_ERROR_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_INTEROP_ERROR_H_

namespace ark::ets::interop::js {

/**
 * Error code format: 109MMSSS
 *
 * - 109: Fixed prefix for ETS-JS interop errors
 *
 * - MM: Major category (01–06):
 *     01: Argument validation errors (input stage)
 *     02: Type system and type conversion errors
 *     03: Method invocation and field access errors
 *     04: Class, module, and VM context errors
 *     05: ANI / N-API interaction errors
 *     06: Runtime and internal system errors
 *
 * - SSS: Sequential identifier within the category (001–999)
 */
enum InteropStatus {
    INTEROP_OK = 0,

    // Number of arguments provided does not match function declaration
    INTEROP_BAD_ARGUMENTS_COUNT = 10901001,
    // Argument type is incompatible with expected type
    INTEROP_ARGUMENT_TYPE_MISMATCH = 10901002,
    // Attempted to access parameter index outside valid range
    INTEROP_PARAMETER_INDEX_OUT_OF_RANGE = 10901003,
    // Function parameter signature does not match expected definition
    INTEROP_FUNCTION_SIGNATURE_MISMATCH = 10901004,
    // Argument value is invalid or violates constraints
    INTEROP_INVALID_ARGUMENT_VALUE = 10901005,

    // Source type is not compatible with target type for assignment
    INTEROP_TYPE_NOT_ASSIGNABLE = 10902001,
    // Conversion from source type to target type is not supported
    INTEROP_UNSUPPORTED_TYPE_CONVERSION = 10902002,
    // Encountered unexpected undefined value where a valid value is required
    INTEROP_UNDEFINED_VALUE = 10902003,
    // Failed to convert null or undefined to object type
    INTEROP_CANNOT_CONVERT_NULL_OR_UNDEFINED_TO_OBJECT = 10902004,
    // Tuple index access is out of bounds
    INTEROP_TUPLE_INDEX_OUT_OF_BOUNDS = 10902005,
    // Unsupported primitive array type conversion
    INTEROP_UNSUPPORTED_PRIMITIVE_ARRAY_TYPE = 10902006,

    // Specified method not found on target object
    INTEROP_METHOD_NOT_FOUND = 10903001,
    // Call target is not a function
    INTEROP_CALL_TARGET_IS_NOT_A_FUNCTION = 10903002,
    // Method overload resolution not supported
    INTEROP_UNSUPPORTED_METHOD_OVERLOAD = 10903003,
    // Failed to access object field
    INTEROP_FIELD_ACCESS_FAILED = 10903004,
    // Error occurred during callback function execution
    INTEROP_CALLBACK_EXECUTION_FAILED = 10903005,

    // Specified class not found
    INTEROP_CLASS_NOT_FOUND = 10904001,
    // Failed to load JavaScript/ETS module
    INTEROP_MODULE_LOAD_FAILED = 10904002,
    // Failed to create proxy object
    INTEROP_PROXY_CREATION_FAILED = 10904003,
    // Static context not properly loaded
    INTEROP_STATIC_CONTEXT_NOT_LOADED = 10904004,
    // Class definition for dynamic function not found
    INTEROP_DYNAMIC_FUNCTION_CLASS_NOT_FOUND = 10904005,
    // Virtual machine handshake failed
    INTEROP_VM_HANDSHAKE_FAILED = 10904006,
    // Proxy object is not extensible
    INTEROP_PROXY_NOT_EXTENSIBLE = 10904007,
    // Violated builtin class constraint
    INTEROP_BUILTIN_CLASS_CONSTRAINT_VIOLATION = 10904008,
    // Shared reference (cross-runtime reference) not found
    INTEROP_SHARED_REFERENCE_NOT_FOUND = 10904009,
    // Virtual machine instance not found
    INTEROP_VM_NOT_FOUND = 10904010,

    // Unknown ANI (Ark Native Interface) error occurred
    INTEROP_UNKNOWN_ANI_ERROR_OCCURRED = 10905001,
    // ANI call failed
    INTEROP_ANI_CALL_FAILED = 10905002,
    // Failed to find class
    INTEROP_FIND_CLASS_FAILED = 10905003,
    // Failed to find namespace
    INTEROP_FIND_NAMESPACE_FAILED = 10905004,
    // Failed to find enum
    INTEROP_FIND_ENUM_FAILED = 10905005,
    // N-API (Node-API) error occurred
    INTEROP_NAPI_ERROR_OCCURRED = 10905006,

    // Memory allocation failed
    INTEROP_MEMORY_ALLOCATION_FAILED = 10906001,
    // Failed to create function object
    INTEROP_FUNCTION_CREATION_FAILED = 10906002,
    // Failed to create Promise object
    INTEROP_PROMISE_CREATION_FAILED = 10906003,
    // Interop call stack allocation failed
    INTEROP_INTEROP_CALL_STACK_ALLOCATION_FAILED = 10906004,
    // Encountered unsupported JavaScript value type
    INTEROP_UNSUPPORTED_JSVALUE_TYPE = 10906005,
    // ETS virtual machine internal error
    INTEROP_ETSVM_INTERNAL_ERROR = 10906006,
    // Thread not attached to virtual machine
    INTEROP_THREAD_NOT_ATTACHED = 10906007,
    // Failed to create object
    INTEROP_OBJECT_CREATION_FAILED = 10906008,
    // JavaScript call signature is invalid
    INTEROP_JSCALL_SIGNATURE_INVALID = 10906009,
    // Call bridge signature is invalid
    INTEROP_CALL_BRIDGE_SIGNATURE_INVALID = 10906010,
    // Call stack is empty
    INTEROP_CALL_STACK_EMPTY = 10906011,
    // Call stack overflow
    INTEROP_CALL_STACK_OVERFLOW = 10906012,
    // Feature not yet implemented
    INTEROP_FEATURE_NOT_IMPLEMENTED = 10906013,
    // Invalid interop context
    INTEROP_INVALID_INTEROP_CONTEXT = 10906014,
    // Shared reference integrity check failed
    INTEROP_SHARED_REFERENCE_INTEGRITY_FAILED = 10906015
};

}  // namespace ark::ets::interop::js

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_INTEROP_ERROR_H_