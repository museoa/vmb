opcodes = Array.new
1.upto(401) do
 opcodes.push("")
end
opcodes[040]+= "|ADD"
opcodes[041]+= "|ADDU"
opcodes[044]+= "|AND"
opcodes[015]+= "|BREAK"
opcodes[032]+= "|DIV"
opcodes[033]+= "|DIVU"
opcodes[011]+= "|JALR"
opcodes[010]+= "|JR"
opcodes[020]+= "|MFHI"
opcodes[022]+= "|MFLO"
opcodes[021]+= "|MTHI"
opcodes[023]+= "|MTLO"
opcodes[030]+= "|MULT"
opcodes[031]+= "|MULTU"
opcodes[047]+= "|NOR"
opcodes[045]+= "|OR"
opcodes[000]+= "|SLL"
opcodes[004]+= "|SLLV"
opcodes[052]+= "|SLT"
opcodes[053]+= "|SLTU"
opcodes[003]+= "|SRA"
opcodes[007]+= "|SRAV"
opcodes[002]+= "|SRL"
opcodes[006]+= "|SRLV"
opcodes[042]+= "|SUB"
opcodes[043]+= "|SUBU"
opcodes[014]+= "|SYSCALL"
opcodes[046]+= "|XOR"
opcodes[010]+= "|ADDI"
opcodes[011]+= "|ADDIU"
opcodes[014]+= "|ANDI"
opcodes[017]+= "|LUI"
opcodes[015]+= "|ORI"
opcodes[012]+= "|SLTI"
opcodes[013]+= "|SLTIU"
opcodes[016]+= "|XORI"
opcodes[004]+= "|BEQ"
opcodes[001]+= "|BGL"
opcodes[001]+= "|BGEZ"
opcodes[021]+= "|BGEZAL"
opcodes[000]+= "|BLTZ"
opcodes[020]+= "|BLTZAL"
opcodes[007]+= "|BGTZ"
opcodes[006]+= "|BLEZ"
opcodes[005]+= "|BNE"
opcodes[002]+= "|J"
opcodes[003]+= "|JAL"
opcodes[020]+= "|COP0SEL"
opcodes[020]+= "|CO0"
opcodes[020]+= "|RFE"
opcodes[010]+= "|TLBP"
opcodes[001]+= "|TLBR"
opcodes[002]+= "|TLBWI"
opcodes[006]+= "|TLBWR"
opcodes[010]+= "|BC0"
opcodes[0400]+= "|BC0F"
opcodes[0401]+= "|BC0T"
opcodes[0]+= "|MFC0"
opcodes[04]+= "|MTC0"
opcodes[040]+= "|LB"
opcodes[044]+= "|LBU"
opcodes[041]+= "|LH"
opcodes[045]+= "|LHU"
opcodes[043]+= "|LW"
opcodes[042]+= "|LWL"
opcodes[046]+= "|LWR"
opcodes[050]+= "|SB"
opcodes[051]+= "|SH"
opcodes[053]+= "|SW"
opcodes[052]+= "|SWL"
opcodes[056]+= "|SWR"

while opcode = gets
  puts opcodes[opcode.to_i]
end