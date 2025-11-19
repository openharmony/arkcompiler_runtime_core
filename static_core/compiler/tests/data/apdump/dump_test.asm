# Copyright (c) 2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# source binary: ./dump_test/dump_test.abc

.language eTS

.record dump_test.Adder <ets.implements=dump_test.Worker, ets.extends=std.core.Object, access.record=public> {
}

.record dump_test.Doubler <ets.implements=dump_test.Worker, ets.extends=std.core.Object, access.record=public> {
}

.record dump_test.ETSGLOBAL <ets.abstract, ets.extends=std.core.Object, access.record=public, ets.annotation.type=class, ets.annotation.class=ets.annotation.Module, ets.annotation.id=id_2051, ets.annotation.element.name=exported, ets.annotation.element.type=array, ets.annotation.element.array.component.type=string> {
}

.record dump_test.Worker <ets.abstract, ets.interface, ets.extends=std.core.Object, access.record=public> {
}

.record escompat.Array <external>

.record escompat.Error <external>

.record ets.annotation.Module <external>

.record std.core.ClassCastError <external>

.record std.core.Console <external>

.record std.core.ETSGLOBAL <external> {
	std.core.Console console <static, external, access.field=public>
}

.record std.core.Object <external>

.record std.core.Runtime <external>

.record std.core.String <external>

.record std.core.StringBuilder <external>

.function void dump_test.ETSGLOBAL._cctor_() <cctor, static> {
	return.void
}

.function f64 dump_test.ETSGLOBAL.foo(f64 a0) <static, access.function=public> {
	fldai.64 0x0
	fcmpl.64 a0
	jlez jump_label_0
	lda.str "neg"
	sta.obj v0
	initobj.short escompat.Error._ctor_:(escompat.Error,std.core.String), v0
	sta.obj v0
	throw v0
jump_label_0:
	movi v0, 0x0
	movi v2, 0x2
	newarr v1, v2, dump_test.Worker[]
	initobj.short dump_test.Adder._ctor_:(dump_test.Adder)
	starr.obj v1, v0
	initobj.short dump_test.Doubler._ctor_:(dump_test.Doubler)
	movi v2, 0x1
	starr.obj v1, v2
	lenarr v1
	sta v2
	initobj.short escompat.Array._ctor_:(escompat.Array,i32), v2
	sta.obj v2
jump_label_2:
	lenarr v1
	jle v0, jump_label_1
	lda v0
	ldarr.obj v1
	call.virt.acc escompat.Array.$_set:(escompat.Array,i32,std.core.Object), v2, v0, v0, 0x2
	inci v0, 0x1
	jmp jump_label_2
jump_label_1:
	call.short dump_test.ETSGLOBAL.noisyBranch:(f64), a0
	call.acc.short dump_test.ETSGLOBAL.runPolymorphic:(escompat.Array,f64), v2, 0x1
	return.64
}

.function void dump_test.ETSGLOBAL.main() <static, access.function=public> {
try_begin_label_0:
	fldai.64 0x4024000000000000
	call.acc.short dump_test.ETSGLOBAL.foo:(f64), v0, 0x0
try_end_label_0:
	jmp jump_label_3
handler_begin_label_0_0:
	sta.obj v0
	movi v1, 0x0
	ldstatic.obj std.core.ETSGLOBAL.console
	sta.obj v2
	movi v4, 0x2
	newarr v3, v4, std.core.Object[]
	lda.str "catch error"
	starr.obj v3, v1
	movi v4, 0x1
	lda.obj v0
	starr.obj v3, v4
	lenarr v3
	sta v0
	initobj.short escompat.Array._ctor_:(escompat.Array,i32), v0
	sta.obj v0
jump_label_5:
	lenarr v3
	jle v1, jump_label_4
	lda v1
	ldarr.obj v3
	call.virt.acc escompat.Array.$_set:(escompat.Array,i32,std.core.Object), v0, v1, v0, 0x2
	inci v1, 0x1
	jmp jump_label_5
jump_label_4:
	call.short std.core.Console.log:(std.core.Console,escompat.Array), v2, v0
jump_label_3:
	return.void

.catch escompat.Error, try_begin_label_0, try_end_label_0, handler_begin_label_0_0
}

.function f64 dump_test.ETSGLOBAL.noisyBranch(f64 a0) <static, access.function=public> {
	movi v0, 0x1
	movi v1, 0x0
	mov v2, v1
jump_label_5:
	lda v2
	i32tof64
	sta.64 v3
	lda.64 a0
	fcmpl.64 v3
	jlez jump_label_2
	lda v2
	modi 0x3
	jnez jump_label_3
	addv v1, v2
	jmp jump_label_4
jump_label_3:
	lda v2
	modi 0x3
	jne v0, try_begin_label_0
	sub v1, v2
	sta v1
	jmp jump_label_4
try_begin_label_0:
	lda.str "bang"
	sta.obj v3
	initobj.short escompat.Error._ctor_:(escompat.Error,std.core.String), v3
	sta.obj v3
	throw v3
try_end_label_0:
	inci v1, 0x0
jump_label_4:
	inci v2, 0x1
	jmp jump_label_5
jump_label_2:
	lda v1
	i32tof64
	return.64

.catch escompat.Error, try_begin_label_0, try_end_label_0, try_end_label_0
}

.function f64 dump_test.ETSGLOBAL.runPolymorphic(escompat.Array a0, f64 a1) <static, access.function=public> {
	movi v0, 0x0
	mov v1, v0
	mov v2, v0
jump_label_2:
	lda v2
	i32tof64
	sta.64 v3
	lda.64 a1
	fcmpl.64 v3
	jlez jump_label_0
	call.virt.short escompat.Array.%%get-length:(escompat.Array), a0
	sta v3
	mod v2, v3
	call.virt.acc.short escompat.Array.$_get:(escompat.Array,i32), a0, 0x1
	sta.obj v3
	jeqz.obj jump_label_1
	lda.obj v3
	checkcast dump_test.Worker
	lda v1
	i32tof64
	sta.64 v1
	lda v2
	i32tof64
	call.virt.acc.short dump_test.Worker.process:(dump_test.Worker,f64), v3, 0x1
	fadd2.64 v1
	f64toi32
	sta v1
	inci v2, 0x1
	jmp jump_label_2
jump_label_1:
	lda.str "Worker"
	call.acc std.core.Runtime.failedTypeCastException:(std.core.Object,std.core.String,u1), v3, v0, v0, 0x1
	sta.obj v0
	throw v0
jump_label_0:
	lda v1
	i32tof64
	return.64
}

.function std.core.ClassCastError std.core.Runtime.failedTypeCastException(std.core.Object a0, std.core.String a1, u1 a2) <native, external, static, access.function=public>

.function void dump_test.Adder._ctor_(dump_test.Adder a0) <ctor, access.function=public> {
	call.short std.core.Object._ctor_:(std.core.Object), a0
	return.void
}

.function f64 dump_test.Adder.process(dump_test.Adder a0, f64 a1) <access.function=public> {
	fldai.64 0x3ff0000000000000
	fadd2.64 a1
	return.64
}

.function void dump_test.Doubler._ctor_(dump_test.Doubler a0) <ctor, access.function=public> {
	call.short std.core.Object._ctor_:(std.core.Object), a0
	return.void
}

.function f64 dump_test.Doubler.process(dump_test.Doubler a0, f64 a1) <access.function=public> {
	fldai.64 0x4000000000000000
	fmul2.64 a1
	return.64
}

.function f64 dump_test.Worker.process(dump_test.Worker a0, f64 a1) <ets.abstract, noimpl, access.function=public>

.function std.core.Object escompat.Array.$_get(escompat.Array a0, i32 a1) <native, external, access.function=public>

.function void escompat.Array.$_set(escompat.Array a0, i32 a1, std.core.Object a2) <native, external, access.function=public>

.function i32 escompat.Array.%%get-length(escompat.Array a0) <external, access.function=public>

.function void escompat.Array._ctor_(escompat.Array a0, i32 a1) <ctor, external, access.function=public>

.function void escompat.Error._ctor_(escompat.Error a0, std.core.String a1) <ctor, external, access.function=public>

.function void std.core.Console.log(std.core.Console a0, escompat.Array a1) <varargs, external, access.function=public>

.function void std.core.Object._ctor_(std.core.Object a0) <ctor, external, access.function=public>
