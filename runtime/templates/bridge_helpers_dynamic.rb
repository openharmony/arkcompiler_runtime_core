# Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

def check_N_v8_format(insn)
  insn.operands.each do |op|
    raise "Instruction " + insn.mnemonic + " has unexpected format " + insn.format.pretty if ! op.reg? || op.width != 8
  end
end

def get_format_for(insn)
  fmt = insn.format.pretty
  if fmt == "imm4_v4_v4_v4_v4_v4"
    # Merge imm4_v4_v4_v4_v4_v4 and imm4_v4_v4_v4 since they haave the same handling code
    fmt = "imm4_v4_v4_v4"
  end

  if insn.prefix
    if fmt == "pref_imm16_v8"
      # This format requires specific handler for each call instruction
      fmt = insn.opcode
    else
      # All other instructions accept N v8
      check_N_v8_format(insn)
      prefix_name = insn.prefix.name
      fmt = prefix_name + "_N_v8"
    end
  end
  return "call_#{fmt}"
end

def get_call_insns
  Panda.instructions.select { |insn| insn.properties.include?('call') && insn.properties.include?('dynamic') }
end
