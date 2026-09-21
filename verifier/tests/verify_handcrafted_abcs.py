#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
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

# End-to-end tests of the ark verifier checks using hand-crafted abc files.
# Usage: python3 verify_handcrafted_abcs.py <path-to-ark_verifier>
import argparse
import os
import struct
import subprocess
import sys
import zlib

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from abc_builder import AbcBuilder, uleb

parser = argparse.ArgumentParser()
parser.add_argument("verifier", help="Path to the ark_verifier executable")
parser.add_argument("--keep-files", action="store_true", help="Keep the generated abc files")
args = parser.parse_args()
VERIFIER = args.verifier
OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "handcrafted_abcs")
os.makedirs(OUT_DIR, exist_ok=True)

# literal tags
TAG_STRING = 0x05
TAG_METHOD = 0x06
TAG_GETTER = 0x1a
TAG_METHODAFFILIATE = 0x09
TAG_INTEGER = 0x02
TAG_DOUBLE = 0x04
TAG_LITERALARRAY = 0x18
TAG_BOOL = 0x01
TAG_ACCESSOR = 0x08
TAG_NULLVALUE = 0xff
TAG_LITERALBUFFERINDEX = 0x17
TAG_ARRAY_U8 = 0x0b
TAG_SETTER = 0x1b

# opcodes
OP_LDUNDEFINED = 0
OP_DEFINECLASS = 53
OP_LDOBJBYNAME = 66
OP_MOV = 69
OP_JMP_IMM8 = 77
OP_LDA = 96
OP_STA = 97
OP_LDAI = 98
OP_RETURN = 100
OP_RETURNUNDEFINED = 101
OP_LDEXTERNALMODULEVAR = 126

# annotation element tag byte ('I' for u32 scalar)
TAG_I = ord('I')

results = []


def write_abc(name, data):
    path = os.path.join(OUT_DIR, name)
    with open(path, 'wb') as f:
        f.write(data)
    return path


def fix_checksum(data):
    data = bytearray(data)
    checksum = zlib.adler32(bytes(data[12:])) & 0xFFFFFFFF
    struct.pack_into('<I', data, 8, checksum)
    return bytes(data)


def verify(path):
    proc = subprocess.run([VERIFIER, '--input_file', path], capture_output=True, text=True)
    if proc.returncode < 0 or proc.returncode > 1:
        return False, 'CRASH (returncode=%d): %s' % (proc.returncode, proc.stderr[:200])
    return proc.returncode == 0, proc.stderr


def run_case(name, data, expect_pass):
    path = write_abc(name + '.abc', data)
    ok, err = verify(path)
    status = 'PASS' if ok else 'FAIL'
    expected = 'PASS' if expect_pass else 'FAIL'
    crashed = err.startswith('CRASH')
    verdict = 'OK' if ok == expect_pass and not crashed else 'MISMATCH!!!'
    results.append((name, expected, status, verdict))
    print(f"[{verdict}] {name}: expected {expected}, got {status}" + (" (CRASH!)" if crashed else ""))
    if verdict == 'MISMATCH!!!':
        print('  stderr:', err[:500])


def make_base(extra_records_fn=None, version=(24, 0, 0, 0)):
    """Common skeleton: record 'Ltest;' with func_main_0 returning undefined."""
    b = AbcBuilder(version)
    return b


def build_simple(insns, num_vregs=2, num_args=3, try_blocks=None, version=(24, 0, 0, 0)):
    b = AbcBuilder(version)
    name_main = b.add_string('func_main_0')
    code = b.add_code(num_vregs, num_args, insns, try_blocks)
    b.add_record('Ltest;', methods=[{'name_off': name_main, 'code_off': code}])
    return b, None


def make_ic_slot_abc(slot_imm, ann_slot_number, version=(24, 0, 0, 0)):
    b = AbcBuilder(version)
    name_main = b.add_string('func_main_0')
    name_slot_ann = b.add_string('L_ESSlotNumberAnnotation;')
    name_slot_elem = b.add_string('SlotNumber')
    name_x = b.add_string('x')
    insns = bytes([OP_LDOBJBYNAME, slot_imm]) + struct.pack('<H', 0) + bytes([OP_RETURNUNDEFINED])
    code = b.add_code(2, 3, insns)
    # annotation record will be class index 1
    ann = b.add_annotation(1, [(name_slot_elem, ann_slot_number, TAG_I)])
    b.add_record('Ltest;', methods=[{'name_off': name_main, 'code_off': code, 'annotations': [ann]}])
    b.add_record('L_ESSlotNumberAnnotation;')
    # constant pool: index 0 is the string 'x' (referenced by ldobjbyname)
    return b, name_x


def make_ic_slot_no_ann_abc(slot_imm):
    b = AbcBuilder((24, 0, 0, 0))
    name_main = b.add_string('func_main_0')
    name_x = b.add_string('x')
    insns = bytes([OP_LDOBJBYNAME, slot_imm]) + struct.pack('<H', 0) + bytes([OP_RETURNUNDEFINED])
    code = b.add_code(2, 3, insns)
    b.add_record('Ltest;', methods=[{'name_off': name_main, 'code_off': code}])
    return b, name_x


# ---------------------------------------------------------------- test 1: sanity
def test_sanity():
    b, _ = build_simple(bytes([OP_LDUNDEFINED, OP_RETURNUNDEFINED]))
    data = b.build(pool_entries=[])
    run_case('sanity_ok', data, True)


# ---------------------------------------------------------------- req 1: uninitialized registers
def test_register_init():
    # var-like: read of a never written vreg is allowed (implicit undefined)
    b, _ = build_simple(bytes([OP_LDA, 0x00, OP_RETURNUNDEFINED]))
    data = b.build(pool_entries=[])
    run_case('reginit_never_written_ok', data, True)

    # def only in dead code: read at entry has no defining path -> fail
    # (jmp +9 skips the dead ldai/sta and lands on the lda of v0)
    insns = bytes([
        OP_JMP_IMM8, 0x09,        # jmp +9 -> to the lda (after the dead sta)
        OP_LDAI, 0x01, 0, 0, 0,   # dead: 5 bytes
        OP_STA, 0x00,             # dead: write v0
        OP_LDA, 0x00,             # read v0: not defined on any path
        OP_RETURNUNDEFINED,
    ])
    b, _ = build_simple(insns, num_vregs=1)
    data = b.build(pool_entries=[])
    run_case('reginit_dead_def_fail', data, False)

    # def before use in straight line: ok
    insns = bytes([
        OP_LDAI, 0x01, 0, 0, 0,
        OP_STA, 0x00,
        OP_LDA, 0x00,
        OP_RETURNUNDEFINED,
    ])
    b, _ = build_simple(insns, num_vregs=1)
    data = b.build(pool_entries=[])
    run_case('reginit_def_before_use_ok', data, True)

    # def on a branch: may-analysis -> read after merge is ok
    insns = bytes([
        OP_LDAI, 0x01, 0, 0, 0,
        OP_STA, 0x00,             # v0 defined
        OP_LDA, 0x00,
        OP_RETURNUNDEFINED,
    ])
    b, _ = build_simple(insns, num_vregs=2, num_args=4)
    data = b.build(pool_entries=[])
    run_case('reginit_arg_read_ok', data, True)

    # the analysis size gates: with 16384 registers and 8 instructions the
    # insn_num * regs^2 product exceeds MAX_REGISTER_ANALYSIS_TIME_COMPLEXITY,
    # so the analysis is skipped and the dead path definition above is not
    # flagged -> pass (an untrusted huge method must not blow up the analysis)
    insns = bytes([OP_LDUNDEFINED] * 3) + bytes([
        OP_JMP_IMM8, 0x09,        # jmp +9 -> to the lda (after the dead sta)
        OP_LDAI, 0x01, 0, 0, 0,   # dead: 5 bytes
        OP_STA, 0x00,             # dead: write v0
        OP_LDA, 0x00,             # read v0: not defined on any path
        OP_RETURNUNDEFINED,
    ])
    b, _ = build_simple(insns, num_vregs=16381, num_args=3)
    data = b.build(pool_entries=[])
    run_case('reginit_time_gate_skip_ok', data, True)

    # the register analysis must stay bounded for the maximum register number:
    # 65536 registers and 256 instructions hit the memory gate boundary
    # exactly (0x1000000) and exceed the time complexity gate, the method
    # still verifies (and does not hang) without the analysis
    insns = bytes([OP_LDUNDEFINED] * 255) + bytes([OP_RETURNUNDEFINED])
    b, _ = build_simple(insns, num_vregs=65533, num_args=3)
    data = b.build(pool_entries=[])
    run_case('reginit_max_regs_dos_shape_ok', data, True)


# ---------------------------------------------------------------- req 2: module instructions
def test_module_instructions():
    def make(with_esmodule_record, module_insn):
        b = AbcBuilder((24, 0, 0, 0))
        name_main = b.add_string('func_main_0')
        insns = bytes([module_insn, 0x00, OP_LDUNDEFINED, OP_RETURNUNDEFINED])
        code = b.add_code(2, 3, insns)
        if with_esmodule_record:
            name_field = b.add_string('test.js')
            req_str = b.add_string('./dep.js')
            mod = struct.pack('<II', 0, 1) + struct.pack('<I', req_str) + struct.pack('<IIIII', 0, 0, 0, 0, 0)
            mod_off = b.add_literal_array(mod)
            b.add_record('Ltest;', methods=[{'name_off': name_main, 'code_off': code}])
            b.add_record('L_ESModuleRecord;', fields=[(name_field, mod_off, 0)])
        else:
            b.add_record('Ltest;', methods=[{'name_off': name_main, 'code_off': code}])
        return b

    # module instruction in non-esmodule record -> fail
    b = make(False, OP_LDEXTERNALMODULEVAR)
    data = b.build(pool_entries=[])
    run_case('module_insn_nonmodule_fail', data, False)

    # module instruction in a file with _ESModuleRecord -> ok
    b = make(True, OP_LDEXTERNALMODULEVAR)
    data = b.build(pool_entries=[])
    run_case('module_insn_esmodule_ok', data, True)

    # no module instruction -> ok
    b = AbcBuilder((24, 0, 0, 0))
    name_main = b.add_string('func_main_0')
    code = b.add_code(2, 3, bytes([OP_LDAI, 1, 0, 0, 0, OP_RETURN]))
    b.add_record('Ltest;', methods=[{'name_off': name_main, 'code_off': code}])
    data = b.build(pool_entries=[])
    run_case('no_module_insn_ok', data, True)


# ---------------------------------------------------------------- req 3: TLA func_main_0
def test_tla_return():
    def make(tla, main_insn):
        b = AbcBuilder((24, 0, 0, 0))
        name_main = b.add_string('func_main_0')
        insns = bytes([main_insn] + ([0x01, 0, 0, 0] if main_insn == OP_LDAI else []) + [OP_RETURN])
        code = b.add_code(2, 3, insns)
        if tla:
            name_tla = b.add_string('hasTopLevelAwait')
            b.add_record('Ltest;', methods=[{'name_off': name_main, 'code_off': code}])
            b.add_record('L_HasTopLevelAwait;', fields=[(name_tla, 1, 0)])
        else:
            b.add_record('Ltest;', methods=[{'name_off': name_main, 'code_off': code}])
        return b

    def make_tla(insns):
        b = AbcBuilder((24, 0, 0, 0))
        name_main = b.add_string('func_main_0')
        name_tla = b.add_string('hasTopLevelAwait')
        code = b.add_code(2, 3, insns)
        b.add_record('Ltest;', methods=[{'name_off': name_main, 'code_off': code}])
        b.add_record('L_HasTopLevelAwait;', fields=[(name_tla, 1, 0)])
        return b

    # TLA func_main_0 whose last instruction is returnundefined -> fail
    b = make_tla(bytes([OP_LDAI, 0x01, 0, 0, 0, OP_RETURNUNDEFINED]))
    data = b.build(pool_entries=[])
    run_case('tla_returnundefined_fail', data, False)

    # TLA func_main_0 with a returnundefined on an unreachable (dead code)
    # path skipped by a jump: only reachable returnundefined is rejected -> ok
    b = make_tla(bytes([OP_JMP_IMM8, 0x03, OP_RETURNUNDEFINED, OP_LDUNDEFINED, OP_RETURN]))
    data = b.build(pool_entries=[])
    run_case('tla_dead_returnundefined_ok', data, True)

    # TLA func_main_0 with a reachable returnundefined -> fail
    b = make_tla(bytes([OP_LDUNDEFINED, OP_RETURNUNDEFINED, OP_LDUNDEFINED, OP_RETURN]))
    data = b.build(pool_entries=[])
    run_case('tla_live_returnundefined_fail', data, False)

    # TLA func_main_0 with value return -> ok
    b = make(True, OP_LDAI)
    data = b.build(pool_entries=[])
    run_case('tla_return_value_ok', data, True)

    # non TLA func_main_0 with returnundefined -> ok
    b = make(False, OP_RETURNUNDEFINED)
    data = b.build(pool_entries=[])
    run_case('nontla_returnundefined_ok', data, True)

    # reachable returnundefined via a backward jump must fail (not only forward)
    b = make_tla(bytes([
        OP_LDUNDEFINED, OP_JMP_IMM8, 0x05, OP_RETURNUNDEFINED, OP_JMP_IMM8, 0xFF,
        OP_LDUNDEFINED, OP_RETURN
    ]))
    data = b.build(pool_entries=[])
    run_case('tla_backward_returnundefined_fail', data, False)


# ---------------------------------------------------------------- req 4: IC slot bounds
def test_ic_slot():
    # slot 0 + annotation slot number 2: ok
    b, str_x = make_ic_slot_abc(0, 2)
    data = b.build(pool_entries=[str_x])
    run_case('ic_slot_in_bounds_ok', data, True)

    # slot 3 + annotation slot number 2: fail (one slot instruction, slot+1 > 2)
    b, str_x = make_ic_slot_abc(3, 2)
    data = b.build(pool_entries=[str_x])
    run_case('ic_slot_oob_fail', data, False)

    # slot 0x80 encoded in the 8 bit form (high bit set): the slot operand is
    # read zero extended, 0x80 + 2 <= 0x100 -> ok
    b, str_x = make_ic_slot_abc(0x80, 0x100)
    data = b.build(pool_entries=[str_x])
    run_case('ic_slot_high_bit_ok', data, True)

    # slot 0xff encoded in the 8 bit form with annotation 0xff: 0xff is the
    # sentinel written by the frontend when the 8 bit slot space is exhausted,
    # the ic of the instruction is disabled and the runtime raises the slot
    # count 0xff to 0x100, so the hole slot is covered -> ok
    b, str_x = make_ic_slot_abc(0xFF, 0xFF)
    data = b.build(pool_entries=[str_x])
    run_case('ic_slot_0xff_sentinel_ok', data, True)

    # slot 0xff sentinel with annotation 0x100 (the value the frontend writes
    # after the overflow): the final slot number of the sentinel is 0x100 -> ok
    b, str_x = make_ic_slot_abc(0xFF, 0x100)
    data = b.build(pool_entries=[str_x])
    run_case('ic_slot_0xff_ann_0x100_ok', data, True)

    # slot 0xff sentinel with a small annotation: the profile type info array
    # does not cover the hole slot 0xff -> fail
    b, str_x = make_ic_slot_abc(0xFF, 2)
    data = b.build(pool_entries=[str_x])
    run_case('ic_slot_0xff_no_room_fail', data, False)

    # slot 0xfd (two slot instruction, ldobjbyname) + annotation 0xfe:
    # the runtime raises 0xfe to 0x100, 0xfd + 2 <= 0x100 -> ok
    b, str_x = make_ic_slot_abc(0xFD, 0xFE)
    data = b.build(pool_entries=[str_x])
    run_case('ic_slot_hole_boundary_ok', data, True)

    # slot 0x10 + annotation 0xffff: the runtime clamps to 0xffff + 2 -> ok
    b, str_x = make_ic_slot_abc(0x10, 0xFFFF)
    data = b.build(pool_entries=[str_x])
    run_case('ic_slot_clamped_ann_ok', data, True)

    # no slot number annotation: the runtime allocates zero ic slots, so an ic
    # instruction accesses the profile type info array out of bounds -> fail
    b, str_x = make_ic_slot_no_ann_abc(7)
    data = b.build(pool_entries=[str_x])
    run_case('ic_no_annotation_fail', data, False)

    # no annotation and no ic instructions: nothing to check -> ok
    b = AbcBuilder((24, 0, 0, 0))
    name_main = b.add_string('func_main_0')
    code = b.add_code(2, 3, bytes([OP_LDUNDEFINED, OP_RETURNUNDEFINED]))
    b.add_record('Ltest;', methods=[{'name_off': name_main, 'code_off': code}])
    data = b.build(pool_entries=[])
    run_case('ic_no_annotation_no_ic_ok', data, True)

    # old version (12.0.6.0): ic check disabled even with small annotation
    b, str_x = make_ic_slot_abc(3, 0, version=(12, 0, 6, 0))
    data = b.build(pool_entries=[str_x])
    run_case('ic_old_version_skip_ok', data, True)

    # old version without annotation: ic check disabled -> ok
    b, str_x = make_ic_slot_no_ann_abc(7)
    b.version = (12, 0, 6, 0)
    data = b.build(pool_entries=[str_x])
    run_case('ic_old_version_no_ann_ok', data, True)


# ---------------------------------------------------------------- req 5: nonStaticNum
def test_nonstatic_num():
    def make(non_static_num, extra_items=None):
        b = AbcBuilder((24, 0, 0, 0))
        name_main = b.add_string('func_main_0')
        name_prop = b.add_string('constructor')
        name_slot_elem = b.add_string('SlotNumber')
        insns = bytes([OP_DEFINECLASS, 0x00]) + struct.pack('<H', 0) + struct.pack('<H', 1) + \
            struct.pack('<H', 0x00) + bytes([0x00, OP_LDUNDEFINED, OP_RETURNUNDEFINED])
        code = b.add_code(2, 3, insns)
        # defineclasswithbuffer uses ic slot 0, the annotation is required
        ann = b.add_annotation(1, [(name_slot_elem, 1, TAG_I)])
        # record first to learn the method offset: method items are inline in the record
        rec_off = b.add_record('Ltest;', methods=[{'name_off': name_main, 'code_off': code,
                                                   'annotations': [ann]}])
        b.add_record('L_ESSlotNumberAnnotation;')
        # compute the inline method item offset: record header size
        rec_name = b'Ltest;'
        hdr_size = len(uleb(len('Ltest;') << 1 | 1)) + len(rec_name) + 1 + 4 + 1 + 1 + 1 + 1
        method_off = rec_off + hdr_size
        # literal array: [STRING name][METHOD method][METHODAFFILIATE 0][INTEGER nonStaticNum]
        items = [
            (TAG_STRING, struct.pack('<I', name_prop)),
            (TAG_METHOD, struct.pack('<I', method_off)),
            (TAG_METHODAFFILIATE, struct.pack('<H', 0)),
            (TAG_INTEGER, struct.pack('<I', non_static_num)),
        ]
        if extra_items:
            items.extend(extra_items)
        lit_raw = b.literal_buffer(items)
        lit_off = b.add_literal_array(lit_raw)
        return b, method_off, lit_off

    # nonStaticNum 1 with 1 method entry: bound = (4 - 1 - 1) / 2 = 1 -> ok
    b, method, lit_off = make(1)
    data = b.build(pool_entries=[method, lit_off])
    run_case('nonstatic_ok', data, True)

    # nonStaticNum 5: > 1 -> fail
    b, method, lit_off = make(5)
    data = b.build(pool_entries=[method, lit_off])
    run_case('nonstatic_oob_fail', data, False)

    # negative-like (huge) nonStaticNum -> fail
    b, method, lit_off = make(0xFFFFFFF0)
    data = b.build(pool_entries=[method, lit_off])
    run_case('nonstatic_negative_fail', data, False)


# ---------------------------------------------------------------- req 6: function num_args
def test_num_args():
    # 2 args only, no call type annotation -> lower limit 3 -> fail
    b = AbcBuilder((24, 0, 0, 0))
    name_main = b.add_string('func_main_0')
    code = b.add_code(2, 2, bytes([OP_LDUNDEFINED, OP_RETURNUNDEFINED]))
    b.add_record('Ltest;', methods=[{'name_off': name_main, 'code_off': code}])
    data = b.build(pool_entries=[])
    run_case('numargs_too_few_fail', data, False)

    # 3 args -> ok
    b = AbcBuilder((24, 0, 0, 0))
    name_main = b.add_string('func_main_0')
    code = b.add_code(2, 3, bytes([OP_LDUNDEFINED, OP_RETURNUNDEFINED]))
    b.add_record('Ltest;', methods=[{'name_off': name_main, 'code_off': code}])
    data = b.build(pool_entries=[])
    run_case('numargs_ok', data, True)


# ---------------------------------------------------------------- req 7: secondary opcode
def test_secondary_opcode():
    b, _ = build_simple(bytes([OP_LDUNDEFINED, OP_RETURNUNDEFINED]))
    data = bytearray(b.build(pool_entries=[]))
    # find the instruction sequence (ldundefined=0x00, returnundefined=0x65)
    idx = data.find(bytes([OP_RETURNUNDEFINED]))
    # tamper: insert a wide-prefixed instruction with invalid secondary (0xfd 0x20)
    # opcode bytes at the beginning of instructions: replace ldundefined with 0xfd, next byte 0x20
    code_ins_off = None
    # find code item: instructions start after 4 uleb values; simply scan for 00 65 near the end
    # tamper first instruction byte to 0xfd and second to 0x20 (invalid wide secondary)
    # the code item: [vregs][args][size][tries][insns...]
    # locate by searching for uleb sequence 02 03 02 00 then instructions
    pos = data.find(bytes([0x02, 0x03, 0x02, 0x00, OP_LDUNDEFINED, OP_RETURNUNDEFINED]))
    if pos <= 0:
        raise RuntimeError('failed to locate the code item instruction bytes')
    data[pos + 4] = 0xFD
    data[pos + 5] = 0x20
    run_case('secondary_opcode_invalid_fail', fix_checksum(bytes(data)), False)

    # A valid wide secondary opcode is wide.ldi32 (prefix 0xfd, secondary 0x00).
    # There is no wide.ldundefined; the positive case is covered by other tests.


# ---------------------------------------------------------------- req 8: literal array tag/size
def make_literal_case(lit_raw):
    b = AbcBuilder((24, 0, 0, 0))
    name_main = b.add_string('func_main_0')
    name_slot_elem = b.add_string('SlotNumber')
    insns = bytes([OP_DEFINECLASS, 0x00]) + struct.pack('<H', 0) + struct.pack('<H', 1) + \
        struct.pack('<H', 0x00) + bytes([0x00, OP_LDUNDEFINED, OP_RETURNUNDEFINED])
    code = b.add_code(2, 3, insns)
    # defineclasswithbuffer uses ic slot 0, the annotation is required
    ann = b.add_annotation(1, [(name_slot_elem, 1, TAG_I)])
    rec_off = b.add_record('Ltest;', methods=[{'name_off': name_main, 'code_off': code,
                                               'annotations': [ann]}])
    b.add_record('L_ESSlotNumberAnnotation;')
    rec_name = b'Ltest;'
    hdr_size = len(uleb(len('Ltest;') << 1 | 1)) + len(rec_name) + 1 + 4 + 1 + 1 + 1 + 1
    method_off = rec_off + hdr_size
    lit_off = b.add_literal_array(lit_raw)
    return b, method_off, lit_off


def test_literal_valid():
    # valid class literal buffer: [STRING][METHOD][METHODAFFILIATE][INTEGER 1]
    b = AbcBuilder((24, 0, 0, 0))
    name_main = b.add_string('func_main_0')
    name_prop = b.add_string('constructor')
    name_slot_elem = b.add_string('SlotNumber')
    insns = bytes([OP_DEFINECLASS, 0x00]) + struct.pack('<H', 0) + struct.pack('<H', 1) + \
        struct.pack('<H', 0x00) + bytes([0x00, OP_LDUNDEFINED, OP_RETURNUNDEFINED])
    code = b.add_code(2, 3, insns)
    ann = b.add_annotation(1, [(name_slot_elem, 1, TAG_I)])
    rec_off = b.add_record('Ltest;', methods=[{'name_off': name_main, 'code_off': code,
                                               'annotations': [ann]}])
    b.add_record('L_ESSlotNumberAnnotation;')
    rec_name = b'Ltest;'
    hdr_size = len(uleb(len('Ltest;') << 1 | 1)) + len(rec_name) + 1 + 4 + 1 + 1 + 1 + 1
    method_off = rec_off + hdr_size
    lit = AbcBuilder.literal_buffer([
        (TAG_STRING, struct.pack('<I', name_prop)),
        (TAG_METHOD, struct.pack('<I', method_off)),
        (TAG_METHODAFFILIATE, struct.pack('<H', 0)),
        (TAG_INTEGER, struct.pack('<I', 1)),
    ])
    lit_off = b.add_literal_array(lit)
    data = b.build(pool_entries=[method_off, lit_off])
    run_case('literal_ok', data, True)


def test_literal_size():
    lit = bytearray(AbcBuilder.literal_buffer([(TAG_INTEGER, struct.pack('<I', 1))]))
    struct.pack_into('<I', lit, 0, 3)  # odd count
    b, method, lit_off = make_literal_case(bytes(lit))
    data = b.build(pool_entries=[method, lit_off])
    run_case('literal_odd_size_fail', data, False)

    lit = bytearray(AbcBuilder.literal_buffer([(TAG_INTEGER, struct.pack('<I', 1))]))
    struct.pack_into('<I', lit, 0, 100)  # claims 100 items
    b, method, lit_off = make_literal_case(bytes(lit))
    data = b.build(pool_entries=[method, lit_off])
    run_case('literal_size_oob_fail', data, False)

    lit = AbcBuilder.literal_buffer([(0x77, struct.pack('<I', 1))])
    b, method_off, lit_off = make_literal_case(lit)
    data = b.build(pool_entries=[method_off, lit_off])
    run_case('literal_invalid_tag_fail', data, False)


def test_literal_values():
    def make_double_case(double_bits):
        b = AbcBuilder((24, 0, 0, 0))
        name_main = b.add_string('func_main_0')
        name_prop = b.add_string('constructor')
        name_slot_elem = b.add_string('SlotNumber')
        insns = bytes([OP_DEFINECLASS, 0x00]) + struct.pack('<H', 0) + struct.pack('<H', 1) + \
            struct.pack('<H', 0x00) + bytes([0x00, OP_LDUNDEFINED, OP_RETURNUNDEFINED])
        code = b.add_code(2, 3, insns)
        ann = b.add_annotation(1, [(name_slot_elem, 1, TAG_I)])
        rec_off = b.add_record('Ltest;', methods=[{'name_off': name_main, 'code_off': code,
                                                   'annotations': [ann]}])
        b.add_record('L_ESSlotNumberAnnotation;')
        rec_name = b'Ltest;'
        hdr_size = len(uleb(len('Ltest;') << 1 | 1)) + len(rec_name) + 1 + 4 + 1 + 1 + 1 + 1
        method_off = rec_off + hdr_size
        lit = AbcBuilder.literal_buffer([
            (TAG_STRING, struct.pack('<I', name_prop)),
            (TAG_DOUBLE, struct.pack('<Q', double_bits)),
            (TAG_INTEGER, struct.pack('<I', 1)),
        ])
        lit_off = b.add_literal_array(lit)
        return b.build(pool_entries=[method_off, lit_off])

    run_case('literal_impure_nan_fail', make_double_case(0xFFFFFFFFFFFFFFFF), False)
    run_case('literal_pure_nan_ok', make_double_case(0x7FF8000000000000), True)

    lit = AbcBuilder.literal_buffer([
        (TAG_STRING, struct.pack('<I', 50)),
        (TAG_INTEGER, struct.pack('<I', 1)),
    ])
    b, method_off, lit_off = make_literal_case(lit)
    data = b.build(pool_entries=[method_off, lit_off])
    run_case('literal_string_header_offset_fail', data, False)

    lit = AbcBuilder.literal_buffer([
        (TAG_GETTER, struct.pack('<I', 1)),
        (TAG_INTEGER, struct.pack('<I', 1)),
    ])
    b, method_off, lit_off = make_literal_case(lit)
    data = b.build(pool_entries=[method_off, lit_off])
    run_case('literal_getter_invalid_method_fail', data, False)


# ---------------------------------------------------------------- req 9: constant pool id bounds
def test_pool_bounds():
    b, _ = build_simple(bytes([OP_LDUNDEFINED, OP_RETURNUNDEFINED]))
    data = bytearray(b.build(pool_entries=[0x60]))
    # patch the pool entry (last 4 bytes of file are the pool) to file_size (out of bounds)
    struct.pack_into('<I', data, len(data) - 4, len(data))
    run_case('pool_id_oob_fail', fix_checksum(bytes(data)), False)

    # zero pool id -> fail
    b, _ = build_simple(bytes([OP_LDUNDEFINED, OP_RETURNUNDEFINED]))
    data = bytearray(b.build(pool_entries=[0x60]))
    struct.pack_into('<I', data, len(data) - 4, 0)
    run_case('pool_id_zero_fail', fix_checksum(bytes(data)), False)


# ---------------------------------------------------------------- req 10: 16bit cp index oob
def test_cp_index_oob():
    b = AbcBuilder((24, 0, 0, 0))
    name_main = b.add_string('func_main_0')
    name_x = b.add_string('x')
    insns = bytes([OP_LDOBJBYNAME, 0x00]) + struct.pack('<H', 0x0100) + bytes([OP_RETURNUNDEFINED])
    code = b.add_code(2, 3, insns)
    b.add_record('Ltest;', methods=[{'name_off': name_main, 'code_off': code}])
    data = b.build(pool_entries=[name_x])
    run_case('cp_index_oob_fail', data, False)


# ---------------------------------------------------------------- req 11: methodId in record
def test_method_in_record():
    b = AbcBuilder((24, 0, 0, 0))
    name_main = b.add_string('func_main_0')
    name_helper = b.add_string('helper')
    name_x = b.add_string('x')
    # defineclasswithbuffer references method index 1 (a string id, not a real method)
    insns = bytes([OP_DEFINECLASS, 0x00]) + struct.pack('<H', 1) + struct.pack('<H', 2) + \
        struct.pack('<H', 0x00) + bytes([0x00, OP_RETURNUNDEFINED])
    code = b.add_code(2, 3, insns)
    rec_off = b.add_record('Ltest;', methods=[{'name_off': name_main, 'code_off': code}])
    rec_name = b'Ltest;'
    hdr_size = len(uleb(len('Ltest;') << 1 | 1)) + len(rec_name) + 1 + 4 + 1 + 1 + 1 + 1
    method_off = rec_off + hdr_size
    lit = AbcBuilder.literal_buffer([
        (TAG_STRING, struct.pack('<I', name_x)),
        (TAG_METHOD, struct.pack('<I', method_off)),
        (TAG_METHODAFFILIATE, struct.pack('<H', 0)),
        (TAG_INTEGER, struct.pack('<I', 1)),
    ])
    lit_off = b.add_literal_array(lit)
    # pool: [0]=real method (unused), [1]=string 'x' used as a method id, [2]=literal
    data = b.build(pool_entries=[method_off, name_x, lit_off])
    run_case('methodid_not_method_fail', data, False)

    # same instruction referencing the real method (index 0): ok
    b = AbcBuilder((24, 0, 0, 0))
    name_main = b.add_string('func_main_0')
    name_prop = b.add_string('constructor')
    name_slot_elem = b.add_string('SlotNumber')
    insns = bytes([OP_DEFINECLASS, 0x00]) + struct.pack('<H', 0) + struct.pack('<H', 1) + \
        struct.pack('<H', 0x00) + bytes([0x00, OP_RETURNUNDEFINED])
    code = b.add_code(2, 3, insns)
    # defineclasswithbuffer uses ic slot 0, the annotation is required
    ann = b.add_annotation(1, [(name_slot_elem, 1, TAG_I)])
    rec_off = b.add_record('Ltest;', methods=[{'name_off': name_main, 'code_off': code,
                                               'annotations': [ann]}])
    b.add_record('L_ESSlotNumberAnnotation;')
    rec_name = b'Ltest;'
    hdr_size = len(uleb(len('Ltest;') << 1 | 1)) + len(rec_name) + 1 + 4 + 1 + 1 + 1 + 1
    method_off = rec_off + hdr_size
    lit = AbcBuilder.literal_buffer([
        (TAG_STRING, struct.pack('<I', name_prop)),
        (TAG_METHOD, struct.pack('<I', method_off)),
        (TAG_METHODAFFILIATE, struct.pack('<H', 0)),
        (TAG_INTEGER, struct.pack('<I', 1)),
    ])
    lit_off = b.add_literal_array(lit)
    data = b.build(pool_entries=[method_off, lit_off])
    run_case('methodid_in_record_ok', data, True)


# ---------------------------------------------------------------- req 12: catchblock type_idx
def test_catch_type_idx():
    # catchall: type encoded as 0 -> decoded 0xFFFFFFFF -> ok
    b, _ = build_simple(
        bytes([OP_LDAI, 0x01, 0, 0, 0, OP_STA, 0x00, OP_LDUNDEFINED, OP_RETURNUNDEFINED]),
        try_blocks=[(0, 5, [(0, 8, 0)])])
    data = b.build(pool_entries=[])
    run_case('catch_type_catchall_ok', data, True)

    # typed catch with encoded 1 -> decoded type_idx 0 -> class index 0 (the
    # Ltest; record), a legal typed catch -> ok
    b, _ = build_simple(
        bytes([OP_LDAI, 0x01, 0, 0, 0, OP_STA, 0x00, OP_LDUNDEFINED, OP_RETURNUNDEFINED]),
        try_blocks=[(0, 5, [(1, 8, 0)])])
    data = b.build(pool_entries=[])
    run_case('catch_type_typed_ok', data, True)

    # typed catch with encoded 2 -> decoded type_idx 1, the class index holds
    # a single class -> out of bounds -> fail
    b, _ = build_simple(
        bytes([OP_LDAI, 0x01, 0, 0, 0, OP_STA, 0x00, OP_LDUNDEFINED, OP_RETURNUNDEFINED]),
        try_blocks=[(0, 5, [(2, 8, 0)])])
    data = b.build(pool_entries=[])
    run_case('catch_type_oob_fail', data, False)


# ---------------------------------------------------------------- req 13: module record
def make_module_abc(num_requests, regular_imports=None, bad_request_idx=False, bad_string=False):
    b = AbcBuilder((24, 0, 0, 0))
    name_main = b.add_string('func_main_0')
    name_field = b.add_string('moduleRecordIdx')
    # module record buffer: [literalnum][num_requests][request string offsets...][regular_import_num]...
    req_str = b.add_string('./dep.js')
    mod = bytearray()
    mod += struct.pack('<I', 0)  # literalnum
    mod += struct.pack('<I', num_requests)
    for _ in range(num_requests):
        mod += struct.pack('<I', req_str if not bad_string else 0xFFFFFFFF)
    mod += struct.pack('<I', len(regular_imports or []))
    for local, imp, req_idx in (regular_imports or []):
        mod += struct.pack('<I', local) + struct.pack('<I', imp) + struct.pack('<H', req_idx)
    mod += struct.pack('<I', 0)  # namespace import num
    mod += struct.pack('<I', 0)  # local export num
    mod += struct.pack('<I', 0)  # indirect export num
    mod += struct.pack('<I', 0)  # star export num
    mod_off = b.add_literal_array(bytes(mod))
    insns = bytes([OP_LDUNDEFINED, OP_RETURNUNDEFINED])
    code = b.add_code(2, 3, insns)
    # main record with moduleRecordIdx field pointing at the module record literal
    b.add_record('Ltest;', fields=[(name_field, mod_off, 0)],
                 methods=[{'name_off': name_main, 'code_off': code}])
    return b


def test_module_record():
    # valid module record with 1 request, no imports
    b = make_module_abc(1)
    data = b.build(pool_entries=[])
    run_case('module_record_ok', data, True)

    # module_request_idx out of range -> fail
    b = AbcBuilder((24, 0, 0, 0))
    name_main = b.add_string('func_main_0')
    name_field = b.add_string('moduleRecordIdx')
    req_str = b.add_string('./dep.js')
    name_local = b.add_string('a')
    name_imp = b.add_string('b')
    mod = bytearray()
    mod += struct.pack('<I', 0)
    mod += struct.pack('<I', 1)
    mod += struct.pack('<I', req_str)
    mod += struct.pack('<I', 1)
    mod += struct.pack('<I', name_local) + struct.pack('<I', name_imp) + struct.pack('<H', 5)  # bad idx
    mod += struct.pack('<IIIII', 0, 0, 0, 0, 0)
    mod_off = b.add_literal_array(bytes(mod))
    code = b.add_code(2, 3, bytes([OP_LDUNDEFINED, OP_RETURNUNDEFINED]))
    b.add_record('Ltest;', fields=[(name_field, mod_off, 0)],
                 methods=[{'name_off': name_main, 'code_off': code}])
    data = b.build(pool_entries=[])
    run_case('module_record_reqidx_oob_fail', data, False)

    # invalid string offset in requests -> fail
    b = make_module_abc(1, bad_string=True)
    data = b.build(pool_entries=[])
    run_case('module_record_bad_string_fail', data, False)

    # claims many requests without data -> fail (size)
    b = AbcBuilder((24, 0, 0, 0))
    name_main = b.add_string('func_main_0')
    name_field = b.add_string('moduleRecordIdx')
    req_str = b.add_string('./dep.js')
    mod = struct.pack('<II', 0, 100) + struct.pack('<I', req_str) + struct.pack('<IIIII', 0, 0, 0, 0, 0)
    mod_off = b.add_literal_array(mod)
    code = b.add_code(2, 3, bytes([OP_LDUNDEFINED, OP_RETURNUNDEFINED]))
    b.add_record('Ltest;', fields=[(name_field, mod_off, 0)],
                 methods=[{'name_off': name_main, 'code_off': code}])
    data = b.build(pool_entries=[])
    run_case('module_record_size_oob_fail', data, False)


def test_literal_extra_tags():
    lit = AbcBuilder.literal_buffer([
        (TAG_BOOL, bytes([1])),
        (TAG_ACCESSOR, bytes([0])),
        (TAG_NULLVALUE, bytes([0])),
        (TAG_INTEGER, struct.pack('<I', 1)),
    ])
    b, method_off, lit_off = make_literal_case(lit)
    data = b.build(pool_entries=[method_off, lit_off])
    run_case('literal_scalar_tags_ok', data, True)

    lit = AbcBuilder.literal_buffer([
        (TAG_LITERALBUFFERINDEX, struct.pack('<I', 0)),
        (TAG_INTEGER, struct.pack('<I', 1)),
    ])
    b, method_off, lit_off = make_literal_case(lit)
    data = b.build(pool_entries=[method_off, lit_off])
    run_case('literal_buffer_index_ok', data, True)

    lit = AbcBuilder.literal_buffer([
        (TAG_SETTER, struct.pack('<I', 1)),
        (TAG_INTEGER, struct.pack('<I', 1)),
    ])
    b, method_off, lit_off = make_literal_case(lit)
    data = b.build(pool_entries=[method_off, lit_off])
    run_case('literal_setter_invalid_method_fail', data, False)


def test_nested_literal_array():
    inner = AbcBuilder.literal_buffer([(TAG_INTEGER, struct.pack('<I', 7))])
    b = AbcBuilder((24, 0, 0, 0))
    name_main = b.add_string('func_main_0')
    name_slot_elem = b.add_string('SlotNumber')
    insns = bytes([OP_DEFINECLASS, 0x00]) + struct.pack('<H', 0) + struct.pack('<H', 1) + \
        struct.pack('<H', 0x00) + bytes([0x00, OP_LDUNDEFINED, OP_RETURNUNDEFINED])
    code = b.add_code(2, 3, insns)
    ann = b.add_annotation(1, [(name_slot_elem, 1, TAG_I)])
    rec_off = b.add_record('Ltest;', methods=[{'name_off': name_main, 'code_off': code,
                                               'annotations': [ann]}])
    b.add_record('L_ESSlotNumberAnnotation;')
    rec_name = b'Ltest;'
    hdr_size = len(uleb(len('Ltest;') << 1 | 1)) + len(rec_name) + 1 + 4 + 1 + 1 + 1 + 1
    method_off = rec_off + hdr_size
    inner_off = b.add_literal_array(inner)
    outer = AbcBuilder.literal_buffer([
        (TAG_LITERALARRAY, struct.pack('<I', inner_off)),
        (TAG_INTEGER, struct.pack('<I', 1)),
    ])
    outer_off = b.add_literal_array(outer)
    data = b.build(pool_entries=[method_off, outer_off])
    run_case('literal_nested_ok', data, True)

    bad_outer = AbcBuilder.literal_buffer([
        (TAG_LITERALARRAY, struct.pack('<I', 0xFFFFFF00)),
        (TAG_INTEGER, struct.pack('<I', 1)),
    ])
    b, method_off, lit_off = make_literal_case(bad_outer)
    data = b.build(pool_entries=[method_off, lit_off])
    run_case('literal_nested_oob_fail', data, False)


def test_array_u8_literal():
    payload = bytearray()
    payload += struct.pack('<I', 2)
    payload.append(TAG_ARRAY_U8)
    payload += struct.pack('<I', 4)
    payload += bytes([1, 2, 3, 4])
    b, method_off, lit_off = make_literal_case(bytes(payload))
    data = b.build(pool_entries=[method_off, lit_off])
    run_case('literal_array_u8_ok', data, True)

    truncated = bytearray()
    truncated += struct.pack('<I', 2)
    truncated.append(TAG_ARRAY_U8)
    truncated += struct.pack('<I', 16)
    truncated += bytes([1, 2])
    b, method_off, lit_off = make_literal_case(bytes(truncated))
    data = b.build(pool_entries=[method_off, lit_off])
    run_case('literal_array_u8_oob_fail', data, False)


def test_ic_slot_alias():
    b = AbcBuilder((24, 0, 0, 0))
    name_main = b.add_string('func_main_0')
    name_slot_ann = b.add_string('L_ESSlotNumberAnnotation;')
    name_slot_elem = b.add_string('SlotNumber')
    name_x = b.add_string('x')
    name_y = b.add_string('y')
    insns = (bytes([OP_LDOBJBYNAME, 0x00]) + struct.pack('<H', 0) +
             bytes([OP_LDOBJBYNAME, 0x00]) + struct.pack('<H', 1) +
             bytes([OP_RETURNUNDEFINED]))
    code = b.add_code(2, 3, insns)
    ann = b.add_annotation(1, [(name_slot_elem, 4, TAG_I)])
    b.add_record('Ltest;', methods=[{'name_off': name_main, 'code_off': code, 'annotations': [ann]}])
    b.add_record('L_ESSlotNumberAnnotation;')
    data = b.build(pool_entries=[name_x, name_y])
    run_case('ic_slot_alias_fail', data, False)


def test_num_args_extra():
    b = AbcBuilder((24, 0, 0, 0))
    name_main = b.add_string('func_main_0')
    code = b.add_code(2, 4, bytes([OP_LDUNDEFINED, OP_RETURNUNDEFINED]))
    b.add_record('Ltest;', methods=[{'name_off': name_main, 'code_off': code}])
    data = b.build(pool_entries=[])
    run_case('numargs_four_ok', data, True)

    b = AbcBuilder((24, 0, 0, 0))
    name_main = b.add_string('func_main_0')
    code = b.add_code(2, 0, bytes([OP_LDUNDEFINED, OP_RETURNUNDEFINED]))
    b.add_record('Ltest;', methods=[{'name_off': name_main, 'code_off': code}])
    data = b.build(pool_entries=[])
    run_case('numargs_zero_fail', data, False)


def test_try_handler_oob():
    b, _ = build_simple(
        bytes([OP_LDAI, 0x01, 0, 0, 0, OP_STA, 0x00, OP_LDUNDEFINED, OP_RETURNUNDEFINED]),
        try_blocks=[(0, 5, [(0, 0x7F, 0)])])
    data = b.build(pool_entries=[])
    run_case('catch_handler_pc_oob_fail', data, False)


def test_ic_version_boundary():
    b, str_x = make_ic_slot_abc(3, 0, version=(23, 0, 0, 0))
    data = b.build(pool_entries=[str_x])
    run_case('ic_version_23_skip_ok', data, True)

    b, str_x = make_ic_slot_abc(0, 2, version=(24, 0, 0, 1))
    data = b.build(pool_entries=[str_x])
    run_case('ic_version_24_0_0_1_ok', data, True)


def test_mov_defines_register():
    insns = bytes([
        OP_LDAI, 0x01, 0, 0, 0,
        OP_STA, 0x00,
        OP_MOV, 0x01, 0x00,
        OP_LDA, 0x01,
        OP_RETURNUNDEFINED,
    ])
    b, _ = build_simple(insns, num_vregs=2)
    data = b.build(pool_entries=[])
    run_case('reginit_mov_ok', data, True)


def test_module_record_empty_requests():
    b = make_module_abc(0)
    data = b.build(pool_entries=[])
    run_case('module_record_zero_requests_ok', data, True)


def test_prefix_opcodes():
    b, _ = build_simple(bytes([OP_LDUNDEFINED, OP_RETURNUNDEFINED]))
    raw = b.build(pool_entries=[])
    data = bytearray(raw)
    pos = data.find(bytes([0x02, 0x03, 0x02, 0x00, OP_LDUNDEFINED, OP_RETURNUNDEFINED]))
    if pos <= 0:
        raise RuntimeError('failed to locate the code item instruction bytes')
    data[pos + 4] = 0xFB
    data[pos + 5] = 0x7F
    run_case('callruntime_secondary_invalid_fail', fix_checksum(bytes(data)), False)

    data = bytearray(raw)
    pos = data.find(bytes([0x02, 0x03, 0x02, 0x00, OP_LDUNDEFINED, OP_RETURNUNDEFINED]))
    if pos <= 0:
        raise RuntimeError('failed to locate the code item instruction bytes')
    data[pos + 4] = 0xFE
    data[pos + 5] = 0x20
    run_case('throw_secondary_invalid_fail', fix_checksum(bytes(data)), False)


def test_ic_slot_sequential():
    b, str_x = make_ic_slot_abc(0, 1)
    data = b.build(pool_entries=[str_x])
    run_case('ic_slot_exact_bound_ok', data, True)

    b, str_x = make_ic_slot_abc(1, 1)
    data = b.build(pool_entries=[str_x])
    run_case('ic_slot_end_oob_fail', data, False)

    b, str_x = make_ic_slot_abc(0x7F, 0x80)
    data = b.build(pool_entries=[str_x])
    run_case('ic_slot_0x7f_ok', data, True)


def test_reginit_sta_only():
    insns = bytes([
        OP_LDUNDEFINED,
        OP_STA, 0x00,
        OP_LDA, 0x00,
        OP_RETURNUNDEFINED,
    ])
    b, _ = build_simple(insns, num_vregs=1)
    data = b.build(pool_entries=[])
    run_case('reginit_sta_then_lda_ok', data, True)


def test_num_args_one():
    b = AbcBuilder((24, 0, 0, 0))
    name_main = b.add_string('func_main_0')
    code = b.add_code(1, 1, bytes([OP_LDUNDEFINED, OP_RETURNUNDEFINED]))
    b.add_record('Ltest;', methods=[{'name_off': name_main, 'code_off': code}])
    data = b.build(pool_entries=[])
    run_case('numargs_one_fail', data, False)


def test_tla_jump_over_return():
    def make_tla(insns):
        b = AbcBuilder((24, 0, 0, 0))
        name_main = b.add_string('func_main_0')
        name_tla = b.add_string('hasTopLevelAwait')
        code = b.add_code(2, 3, insns)
        b.add_record('Ltest;', methods=[{'name_off': name_main, 'code_off': code}])
        b.add_record('L_HasTopLevelAwait;', fields=[(name_tla, 1, 0)])
        return b

    b = make_tla(bytes([OP_JMP_IMM8, 0x02, OP_RETURNUNDEFINED, OP_RETURN]))
    data = b.build(pool_entries=[])
    run_case('tla_jump_over_returnundefined_ok', data, True)


def test_module_star_export():
    b = AbcBuilder((24, 0, 0, 0))
    name_main = b.add_string('func_main_0')
    name_field = b.add_string('moduleRecordIdx')
    req_str = b.add_string('./dep.js')
    mod = bytearray()
    mod += struct.pack('<I', 0)
    mod += struct.pack('<I', 1)
    mod += struct.pack('<I', req_str)
    mod += struct.pack('<I', 0)
    mod += struct.pack('<I', 0)
    mod += struct.pack('<I', 0)
    mod += struct.pack('<I', 0)
    mod += struct.pack('<I', 1)
    mod += struct.pack('<H', 0)
    mod_off = b.add_literal_array(bytes(mod))
    code = b.add_code(2, 3, bytes([OP_LDUNDEFINED, OP_RETURNUNDEFINED]))
    b.add_record('Ltest;', fields=[(name_field, mod_off, 0)],
                 methods=[{'name_off': name_main, 'code_off': code}])
    data = b.build(pool_entries=[])
    run_case('module_star_export_ok', data, True)


def main():
    if not os.path.exists(VERIFIER):
        print("ark_verifier not found:", VERIFIER)
        return 2
    test_sanity()
    test_register_init()
    test_module_instructions()
    test_tla_return()
    test_ic_slot()
    test_nonstatic_num()
    test_num_args()
    test_secondary_opcode()
    test_literal_valid()
    test_literal_size()
    test_literal_values()
    test_literal_extra_tags()
    test_nested_literal_array()
    test_array_u8_literal()
    test_ic_slot_alias()
    test_num_args_extra()
    test_try_handler_oob()
    test_ic_version_boundary()
    test_mov_defines_register()
    test_module_record_empty_requests()
    test_prefix_opcodes()
    test_ic_slot_sequential()
    test_reginit_sta_only()
    test_num_args_one()
    test_tla_jump_over_return()
    test_module_star_export()
    test_pool_bounds()
    test_cp_index_oob()
    test_method_in_record()
    test_catch_type_idx()
    test_module_record()
    print('\n=== summary ===')
    mismatches = [r for r in results if r[3] != 'OK']
    print(f'total: {len(results)}, mismatches: {len(mismatches)}')
    for m in mismatches:
        print('  MISMATCH:', m[0])
    if not mismatches and not args.keep_files:
        import shutil
        shutil.rmtree(OUT_DIR)
    return 1 if mismatches else 0


if __name__ == '__main__':
    sys.exit(main())
