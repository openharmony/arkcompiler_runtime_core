/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

/*---
desc: >-
      Module imports top-level declaration - types, classes, interfaces, enums, variables (including const) and functions
      to reexport them.
tags: [compile-only, not-a-test]
files:
- ./declarations.ets
---*/

// classes, interfaces, enum
import {A as RA} from "./declarations.ets"
import {B as RB} from "./declarations.ets"
import {AC as RAC} from "./declarations.ets"
import {C as RC} from "./declarations.ets"
import {I as RI} from "./declarations.ets"
import {II as RII} from "./declarations.ets"
import {D as RD} from "./declarations.ets"
import {ENUM as RENUM} from "./declarations.ets"

// variables
import {a as ra} from "./declarations.ets"
import {i as ri} from "./declarations.ets"
import {n as rn} from "./declarations.ets"
import {s as rs} from "./declarations.ets"
import {c as rc} from "./declarations.ets"
import {u as ru} from "./declarations.ets"
import {arr as rarr} from "./declarations.ets"

// constants
import { ca as rca } from "./declarations.ets"
import { cs as rcs } from "./declarations.ets"
import { ci as rci } from "./declarations.ets"

// functions
import { NoArg as RNoArg } from "./declarations.ets"
import { OneArg as ROneArg } from "./declarations.ets"
import { TwoArgs as RTwoArgs } from "./declarations.ets"
import { Gen as RGen } from "./declarations.ets"
import { ArgFun as RArgFun } from "./declarations.ets"
import { ArgFun2 as RArgFun2 } from "./declarations.ets"

// types
import { CLASS_A as RCLASS_A } from "./declarations.ets"
import { VAR_i as RVAR_i } from "./declarations.ets"
import { TPL as RTPL } from "./declarations.ets"
