/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "runtime/fibers/arch/asm_macros.h"
#include "runtime/fibers/arch/arm/context_layout.h"

.extern AsmUnreachable

/**
 * arguments:
 * r0: uint8_t* ctx_memory
 */
FUNCTION_START(GetCurrentContext)
    blx AsmUnreachable
FUNCTION_END(GetCurrentContext)

/**
 * arguments:
 * r0: uint8_t* ctx_memory_from
 * r1: uint8_t* ctx_memory_to
 */
FUNCTION_START(SwitchContext)
    blx AsmUnreachable
FUNCTION_END(SwitchContext)

/**
 * arguments:
 * r0: uint8_t* ctx_memory
 * r1: void (*func)(void)
 * r2: void* data
 * r3: void* stack
 * r4: size_t stack_size_bytes
 */
FUNCTION_START(UpdateContext)
    blx AsmUnreachable
FUNCTION_END(UpdateContext)

/**
 * arguments:
 * r0: uint8_t* ctx_memory
 * r1: void (*func)(void)
 * r2: void* data
 */
FUNCTION_START(UpdateContextKeepStack)
    blx AsmUnreachable
FUNCTION_END(UpdateContextKeepStack)

// we don't need executable stack.
.section .note.GNU-stack,"",%progbits