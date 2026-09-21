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

# Test utility to hand-craft minimal dynamic abc files for verifier tests.
# Layout follows libpandafile File/ClassDataAccessor/MethodDataAccessor formats.

import struct
import zlib

INVALID = 0xFFFFFFFF
MAGIC = b'PANDA\x00\x00\x00'
HEADER_SIZE = 60


def uleb(v):
    out = bytearray()
    while True:
        b = v & 0x7F
        v >>= 7
        if v:
            out.append(b | 0x80)
        else:
            out.append(b)
            return bytes(out)


def sleb(v):
    out = bytearray()
    more = True
    while more:
        b = v & 0x7F
        v >>= 7
        if (v == 0 and (b & 0x40) == 0) or (v == -1 and (b & 0x40) != 0):
            more = False
        else:
            b |= 0x80
        out.append(b)
    return bytes(out)


class AbcBuilder:
    def __init__(self, version=(24, 0, 0, 0)):
        self.version = version
        # entities must start after sizeof(Header), EntityId::IsValid() requires
        # the offset to be greater than the header size
        self.items_start = HEADER_SIZE + 16
        self.buf = bytearray(self.items_start)
        self.strings = {}       # value -> offset
        self.class_index = []   # offsets of records (class index region)
        self.pool = None        # constant pool entries (method index region)

    @staticmethod
    def literal_buffer(items):
        """Build a literal array payload from tagged value pairs."""
        out = bytearray()
        out += struct.pack('<I', 2 * len(items))
        for tag, value in items:
            out.append(tag)
            out += value
        return bytes(out)

    @staticmethod
    def method_item(class_index, name_off, code_off, annotations=None, access=0):
        """Return raw method item bytes. Offsets are patched later."""
        item = bytearray()
        item += struct.pack('<H', class_index)
        item += struct.pack('<H', 0xFFFF)  # proto idx: invalid
        item += struct.pack('<I', 0)  # name placeholder
        item += uleb(access)
        item += b'\x00'  # code placeholder tag position marker (patched by record)
        item += struct.pack('<I', 0)  # code off placeholder
        return bytes(item), name_off, code_off, (annotations or [])

    def add_string(self, value):
        if value in self.strings:
            return self.strings[value]
        self._pad4()
        off = len(self.buf)
        data = value.encode('utf-8')
        self.buf += uleb((len(value) << 1) | 1) + data + b'\x00'
        self.strings[value] = off
        return off

    def add_annotation(self, class_index, elements):
        """Append an annotation item and return its file offset."""
        self._pad4()
        off = len(self.buf)
        item = bytearray()
        item += struct.pack('<H', class_index)
        item += struct.pack('<H', len(elements))
        for name_off, value, tag in elements:
            item += struct.pack('<I', name_off)
            item += struct.pack('<I', value)
        for _, _, tag in elements:
            item.append(tag)
        self.buf += item
        return off

    def add_code(self, num_vregs, num_args, instructions, try_blocks=None):
        """Append a code item and return its file offset."""
        self._pad4()
        off = len(self.buf)
        tries = try_blocks or []
        item = bytearray()
        item += uleb(num_vregs)
        item += uleb(num_args)
        item += uleb(len(instructions))
        item += uleb(len(tries))
        item += instructions
        for start_pc, length, catches in tries:
            item += uleb(start_pc) + uleb(length) + uleb(len(catches))
            for type_enc, handler_pc, code_size in catches:
                item += uleb(type_enc) + uleb(handler_pc) + uleb(code_size)
        self.buf += item
        return off

    def add_literal_array(self, raw):
        self._pad4()
        off = len(self.buf)
        self.buf += raw
        return off

    def add_record(self, name, methods=None, fields=None, annotations_offsets=None):
        """Append a class record with optional methods and fields."""
        # NOTE: methods and fields are stored inline in the record item, so their
        # referenced items (code, annotations) must be added BEFORE the record.
        self._pad4()
        off = len(self.buf)
        item = bytearray()
        name_bytes = name.encode('utf-8')
        item += uleb(len(name) << 1 | 1) + name_bytes + b'\x00'
        item += struct.pack('<I', 0)  # no super class
        item += uleb(0)  # access flags
        item += uleb(len(fields or []))
        item += uleb(len(methods or []))
        item += b'\x00'  # NOTHING tag, no source lang
        own_class_index = len(self.class_index)
        for name_off, value, type_index in (fields or []):
            item += struct.pack('<H', own_class_index)
            item += struct.pack('<H', type_index)
            item += struct.pack('<I', name_off)
            item += uleb(0)  # field access flags
            item += b'\x01'  # INT_VALUE tag
            item += sleb(value if value < 0x80000000 else value - 0x100000000)
            item += b'\x00'  # NOTHING
        for m in (methods or []):
            item += struct.pack('<H', own_class_index)
            item += struct.pack('<H', 0xFFFF)  # proto idx invalid
            item += struct.pack('<I', m['name_off'])
            item += uleb(m.get('access', 0))
            if m.get('code_off') is not None:
                item += b'\x01' + struct.pack('<I', m['code_off'])
            for ann_off in m.get('annotations', []):
                item += b'\x06' + struct.pack('<I', ann_off)
            item += b'\x00'  # NOTHING
        self.buf += item
        self.class_index.append(off)
        return off

    def build(self, pool_entries):
        """Write header, indexes and checksum. pool_entries are constant pool offsets."""
        # class index array in header
        class_idx_off = len(self.buf)
        for _ in self.class_index:
            self.buf += struct.pack('<I', 0)
        # lnp index: empty
        lnp_idx_off = len(self.buf)
        # index section: header + class index + method index (constant pool)
        index_section_off = len(self.buf)
        class_index_off = len(self.buf) + 40
        class_index_size = len(self.class_index)
        method_index_off = class_index_off + 4 * class_index_size
        method_index_size = len(pool_entries)
        end_offset = len(self.buf) + 40 + 4 * class_index_size + 4 * method_index_size
        hdr = struct.pack('<10I', self.items_start, end_offset, class_index_size, class_index_off,
                          method_index_size, method_index_off, 0, 0, 0, 0)
        self.buf += hdr
        for c in self.class_index:
            self.buf += struct.pack('<I', c)
        for p in pool_entries:
            self.buf += struct.pack('<I', p)

        file_size = len(self.buf)
        version = bytes(self.version)
        # abc versions <= 12.0.6.0 store the literal array index in the header
        literal_arrays_in_header = self.version <= (12, 0, 6, 0)
        num_literalarrays = 0 if literal_arrays_in_header else INVALID
        literalarray_idx_off = lnp_idx_off if literal_arrays_in_header else INVALID
        header = bytearray()
        header += MAGIC
        header += struct.pack('<I', 0)  # checksum
        header += version
        header += struct.pack('<11I', file_size, 0, 0, len(self.class_index), class_idx_off,
                              0, lnp_idx_off, num_literalarrays, literalarray_idx_off, 1,
                              index_section_off)
        self.buf[0:HEADER_SIZE] = header
        # fill class idx array
        for i, c in enumerate(self.class_index):
            struct.pack_into('<I', self.buf, class_idx_off + 4 * i, c)
        checksum = zlib.adler32(bytes(self.buf[12:])) & 0xFFFFFFFF
        struct.pack_into('<I', self.buf, 8, checksum)
        return bytes(self.buf)

    def _pad4(self):
        while len(self.buf) % 4 != 0:
            self.buf.append(0)
