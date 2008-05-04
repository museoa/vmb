#$Id: exec.s,v 1.1 2008-05-04 15:47:00 mbbh Exp $

    .abicalls

 # GCC automatic generated header
 
 # GNU C 2.7.2 [AL 1.1, MM 40] Linux/MIPSEL compiled by GNU C

 # Cc1 defaults:
 # -mgas -mabicalls

 # Cc1 arguments (-G value = 0, Cpu = 3000, ISA = 1):
 # -quiet -dumpbase -version -o
 # 

     # Constants

    # BIOS BREAK codes
    
    FORKBCODE    = 0
    LDSTBCODE    = 1
    PANICBCODE   = 2
    HALTBCODE    = 3

    # miscellaneous

    ASIDMASK     = 0x00000FC0
    ASIDSTEP     = 0x00000040
    GBITONMASK   = 0x00000100
    VPNMASK      = 0xFFFFF000
    BIOSVECTPAGE = 0x20000000
    VECTSIZE     = 140

    # NOP padding: 30 nops - string length (32 bytes) 
    PADDING       = (30 * 4) - 32
    ENDSTRADDR    = 0x00000008
    PANICSTRADDR  = ENDSTRADDR + 16

    TERM0COMMAND  = 0x1000025C
    BUSYCODE      = 3
    BYTELEN       = 8
    PRINTCHR      = 2    

    PTEBUGFLAG    = 0x20000FFC
    PTESTART      = 0x20000500
    PTESEGSIZE    = 12
    PTESEGOFFS    = 30
    PTEMAGIC      = 0x2A
    PTEMAGICOFFS  = 24
    PTECNTOFFS    = 8
    UTLBSTACK     = 0x20000FF8
    
    BADPTE        = 13
    PTEMISS       = 14
    
    CAUSESTART    = 2
    CAUSEMASK     = 0x1F
    CAUSENEGMASK  = 0xFFFFFF83
    KUPSTATUSMASK = 0x00000008
    SAFESTATUSMASK = 0xFEFFFFFC
    CPU0BITPOS    = 28

    RAMBASEADDR   = 0x10000000
    RAMSIZEADDR   = 0x10000004
    
    EXTINTMASK       = 0x0000FF00
    EXTINTSCRCHKMASK = 0x00004000
    EXTINTKBCHKMASK  = 0x00008000
    EXTINTKBDONE     = 0xFFFF7FFF
    EXTINTSCRDONE    = 0xFFFFBFFF
    INTSTORAGE       = 0x1E000000
    
    KEYBBUFFSPTR     = 0x1E000000
    KEYBBUFFGPTR     = 0x1E000004
    KEYBBUFFSTART    = 0x1E000008
    KEYBBUFFEND      = 0x1E00003A
    KEYBDEVID        = 0x1000000C 
    
    SCREENBUFFSPTR   = 0x1E00003E
    SCREENBUFFGPTR   = 0x1E000042
    SCREENBUSYFLAG   = 0x1E000046
    SCREENBUFFSTART  = 0x1E000047
    SCREENBUFFEND    = 0x1E000079
    SCREENDEVID      = 0x1000025C 

    
    #Code start

    .text
    .align   2
    .globl  bios
    .type   bios,@function
    .ent    bios
bios:
    .frame   $fp,0,$27
    .mask    0x00000000,0
    .fmask   0x00000000,0
    .set noat

    # 0x00000000 address
     # this is the entry point for UTLB type exceptions: so it sends 
    # them to its proper local handler
    .set noreorder
    .set nomacro
    j    LUTLBHandler
    nop
EndStr:
    .asciiz    "System halted \n"    
#END
PanicStr:
    .asciiz "kernel panic()\n"
    .space PADDING
#END
    
    # 0x00000080 address: jumps to exception handler
    j    LEXCHandler
    nop
    .set reorder
    .set macro

LPanic:
    # this call prints a message on terminal 0 and loops forever 
    li    $4, PANICSTRADDR - 1
#END

LTermOp:
    li     $7, TERM0COMMAND
    li     $8, BUSYCODE
#END

LWaitForReady:
    lw     $6, -4($7)
    beq    $6, $8, LWaitForReady
#END

LPrintLoop:
    addiu    $4, 1
    lbu    $5, 0($4)

    # char 0 ends the string
    beq    $0, $5, LInfinite
    
    #prepares PRINTCHAR command
    sll     $5, BYTELEN
    addi    $5, PRINTCHR
    sw    $5, 0($7)

    j    LWaitForReady 

#END


LInfinite:
    j    LInfinite
#END

LHalt:
    # this call prints a message on terminal 0 and loops forever 
    li    $4, ENDSTRADDR - 1
    j     LTermOp
#END

LUTLBHandler:
    # this routine handles TLBL/TLBS refill misses
    
    # clears PTEBUGFLAG for extra exceptions signaling
    li    $26, PTEBUGFLAG
    sw  $0,  0($26)
    
    # gets some register work space using bottom of BIOS page frame
    li    $26, UTLBSTACK 
    sw    $4, 0($26)
    sw    $5, -4($26)
    sw    $6, -8($26)
    sw    $7, -12($26)
    
    # save failed VPN + ASID into register $27
    mfc0    $27, $10
    
    #computes ASID in $7
    move $7, $27
    andi $7, ASIDMASK
    
    # gets PTE segment starting address in $6
    li    $6, PTESTART
#END
LPTEScanLoop:
    blez $7, LPTEStartFound
    addi $6, PTESEGSIZE
    addi $7, -ASIDSTEP     
    j LPTEScanLoop
#END
 
LPTEStartFound:
    # PTE segment table for ASID is in $6
    # now looks for correct segment
    
    #using $5 for segment computation
    move    $5, $27
    srl    $5, PTESEGOFFS

    #segments 0 and 1 address is in $6 already
    beq    $0, $5, LPTESegmentFound
    addi    $5, -1
    beq $0, $5, LPTESegmentFound

    # increment $6 for segment 2
    addi    $6, 4
    addi    $5, -1
    beq $0, $5, LPTESegmentFound

    # then it's segment 3    
    addi    $6, 4
#END
LPTESegmentFound:
    
    # in $6 there is the PTE segment starting address
    # check if the PTE address is aligned and between RAMBASE and RAMTOP
    lw    $6, 0($6)
    
    #check alignment
    move     $7, $6
    andi     $7, 0x3
    bne     $0, $7, LBadPTE
    
    # check RAMBASE
    li     $7, RAMBASEADDR
    lw     $7, 0($7)
    subu    $7, $6, $7
    bltz     $7, LBadPTE
    
    # check RAMTOP
    li     $7, RAMBASEADDR
    lw     $7, 0($7)
    li     $5, RAMSIZEADDR
    lw     $5, 0($5)
    add    $7, $7, $5
    subu     $7, $7, $6
    blez     $7, LBadPTE
    
    # now check if it is a valid PTE at $6
    lw    $7, 0($6)
    li      $5, PTEMAGIC
    srl     $7, PTEMAGICOFFS
    bne    $5, $7, LBadPTE
    
    #get PTE entry count in $7 
    lw      $7, 0($6)
    # clean up MAGIC tag
    sll    $7, PTECNTOFFS
    srl     $7, PTECNTOFFS

    # check if the PTE ends over RAMTOP
    
    # get RAMTOP
    li     $4, RAMBASEADDR
    lw     $4, 0($4)
    li     $5, RAMSIZEADDR
    lw     $5, 0($5)
    add    $4, $4, $5
    
    #subtract PTEBASE and tag
    subu     $4, $4, $6
    addi    $4, -4
    
    # shift #entries left for multiplying x 8 and get PTESIZE
    sll     $7, 3
    
    #subtract PTESIZE and check
    subu    $4, $7
    blez    $4, LBadPTE
    
    # else is ok
    
    #restore $7
    srl     $7, 3    

    
    # Now scans PTE up to $7 entries, getting them in $5 from $26 address 
    # if the VPN+ASID does not match, checks for G bit and, if on, tries
    # the VPN-only match against $4
    
    #set $26 to first TLB entry 
    move     $26, $6
    addiu     $26, 4

    # set $6 to VPNMASK and $4 to match VPN without ASID
    li    $6, VPNMASK
    and    $4, $27, $6
#END
LSearchLoop:
    lw     $5, 0($26)
    beq    $27, $5, LTLBFound
    
    #checks for G bit
    lw    $5, 4($26)
    andi    $5, GBITONMASK
    beq    $0, $5, LEndLoop
    
    # G bit is on: checks against $4
    lw    $5, 0($26)
    and     $5, $5, $6
    beq    $4, $5, LTLBFound 
#END
LEndLoop:
    addiu    $26, 8
    addi    $7, -1
    beq     $0, $7, LPTEMiss
    j LSearchLoop
#END
LTLBFound:
    # $27 == $5 o G bit entry match
    # load TLB using $5, restore proc status by putting 
    # $27 in ENTRYHI again, reload registers and return thru $26
    lw    $5, 0($26)
    mtc0    $5, $10
    lw    $5, 4($26)
    mtc0    $5, $2

    # this is needed to assure that tlbwr works correctly, since
    # assembler does not see the data dependency between mtc0 and tlb
    # operations
    .set noreorder
    .set nomacro
    nop
    tlbwr
    nop
    .set reorder
    .set macro

    # restore ENTRYHI after TLB write (ASID could be different) 
    mtc0    $27, $10
    
    # reload registers from stack
    li      $26, UTLBSTACK
    lw    $4, 0($26)
    lw    $5, -4($26)
    lw    $6, -8($26)
    lw    $7, -12($26)
    
    #get EPC from CP0
    mfc0    $26, $14

    .set noreorder
    .set nomacro
    jr    $26
    rfe
    .set reorder
    .set macro
#END

LBadPTE:
    # set PTEBUGFLAG to signal TLB exc handler the problem
    li      $26, PTEBUGFLAG
    li    $4, BADPTE
    sw      $4,  0($26)
    j     LTLBNotFound
#END
  
LPTEMiss:
    # set PTEBUGFLAG to signal TLB exc handler the problem and fall through
    li      $26, PTEBUGFLAG
    li    $4, PTEMISS
    sw      $4,  0($26)
#END

LTLBNotFound:
    # table exausted and match not found; reload registers,
    # ENTRYHI (which is still in $27), and fall thru to default exc handler
    mtc0    $27, $10
    lw    $4, 0($26)
    lw    $5, -4($26)
    lw    $6, -8($26)
    lw    $7, -12($26)
#END

LEXCHandler:
    # here starts the general exception handler
    # k0 and k1 registers are always available (never have live
    # values outside BIOS)

    # it must discover the cause of BIOS call, inspecting 
    # the cause register: this is done copying it to $26 and computing
    # the needed value
    mfc0    $26, $13
    srl    $26, CAUSESTART
    andi    $26, $26, CAUSEMASK

    # cause == 0 is interrupt
    beq    $0, $26, LIntHandler
    
    # cause 1-3 is a TLB exception (cause = 2 or 3 and EPC in user
    # space means UTLBHandler has failed)
    addi    $26, -3
    blez    $26, LTLBHandler
    
    #cause 4-7 is a program trap
    addi    $26, -4
    blez    $26, LPRGHandler

    #cause 8 is syscall
    addi    $26, -1
    beq    $0, $26, LSYSHandler

    #cause 9 is break
    addi     $26, -1
    beq    $0, $26, LBreakHandler
    
    # other causes are program trap
    j    LPRGHandler
#END

LIntHandler:
    # if EPC address is in BIOS area, something in kernel
    # has gone orribly wrong: eg. BIOS running with int unmasked
    # (a big bug for BIOS)
    mfc0    $26, $14
    li        $27, BIOSVECTPAGE
    subu    $26, $26, $27
    bltz    $26, LPanic
    # else
    # save processor state into old area and load new state:
    # this is done calling a BIOS subroutine
    li    $27, 0
    j    LSOldLNewArea
#END

LTLBHandler:
    # if EPC address is in BIOS area, something in kernel
    # has gone orribly wrong: eg. page table area corrupted
    # and consequent UTLB handler error
    # (or bug in BIOS)
    mfc0    $26, $14
    li    $27, BIOSVECTPAGE
    subu    $26, $26, $27
    bltz    $26, LPanic    

    li    $27, 2
    j    LSOldLNewArea
#END

LPRGHandler:
    # if EPC address is in BIOS area, something in kernel or in BIOS
    # has gone orribly wrong (a BIOS bug probably)
    mfc0    $26, $14
    li      $27, BIOSVECTPAGE
    subu    $26, $26, $27
    bltz    $26, LPanic

    li    $27, 4
    j    LSOldLNewArea
#END

LSYSHandler:
    # if EPC address is in BIOS area, something in kernel or in BIOS
    # has gone orribly wrong (a BIOS bug probably)
    mfc0    $26, $14
    li      $27, BIOSVECTPAGE
    subu    $26, $26, $27
    bltz    $26, LPanic

    li    $27, 6
    j    LSOldLNewArea
#END

LBreakHandler:
    # if EPC address is in BIOS area, something in kernel or in BIOS
    # has gone orribly wrong (a BIOS bug probably)
    mfc0    $26, $14
    li        $27, BIOSVECTPAGE
    subu        $26, $26, $27
    bltz    $26, LPanic


    # a BIOS service routine is requested:
    # look into $4 register for identification
    
    move    $27, $4

    # any BREAK may be executed only in kernel mode
    # kernel mode of caller is set when KUP bit in STATUS mask is 0
    mfc0    $26, $12
    andi    $26, $26, KUPSTATUSMASK
    beq    $0, $26, LisKernel
    j     LSYSHandler    
#END

LisKernel:
    # 0  is FORK(STATUS, EntryHI, PC, CAUSE)
    beq     $0, $27, LFORK
    
    addi    $27, -1
    
    # 1 is LDST
    beq     $0, $27, LLDST
    
    addi    $27, -1

    # 2 is PANIC routine
    beq    $0, $27, LPanic

    addi    $27, -1
    
    # 3 is HALT routine
    beq     $0, $27, LHalt

    
    # any other break is passed up to SYS handler
    j    LSYSHandler
#END

LFORK:
    # $2 is ENTRYHI, $5 is STATUS, $6 is new PC, $7 is CAUSE
    
    # EntryHI loading
    mtc0    $2, $10
    
    # STATUS preparation
    move     $27, $5
    # this for BIOS safety: no KU, IE or VM bits on
    li    $26, SAFESTATUSMASK
    and    $27, $27, $26

    # STATUS loading
    mtc0    $27, $12    
    
    # CAUSE loading
    mtc0     $7, $13

    # get new PC and jump
    move     $27, $6
    
    .set noreorder
    .set nomacro
    jr    $27
    rfe
    .set reorder
    .set macro
#END

LLDST:
    # this means load from physical address in $5
    move    $26, $5
    j    LLoadStart
#END

LReturnFromBreak:
    # load back PC and jump
    mfc0    $27, $14
    addiu    $27, 4
    .set noreorder
    .set nomacro
    jr    $27
    rfe
    .set reorder
    .set macro    
#END


LSOldLNewArea:
    # this first locates the needed area using $26, then saves the
    # registers and other info
    li    $26, BIOSVECTPAGE
#END
LComputeArea:
    blez    $27, LSave
    addiu    $26, VECTSIZE
    addi    $27, -1
    j     LComputeArea
#END

LSave:
    sw    $1, 16($26)
    sw    $2, 20($26)
    sw    $3, 24($26)
    sw    $4, 28($26)
    sw    $5, 32($26)
    sw    $6, 36($26)
    sw    $7, 40($26)
    sw    $8, 44($26)
    sw    $9, 48($26)
    sw    $10, 52($26)
    sw    $11, 56($26)
    sw    $12, 60($26)
    sw    $13, 64($26)     
    sw    $14, 68($26)
    sw    $15, 72($26)
    sw    $16, 76($26)
    sw    $17, 80($26)
    sw    $18, 84($26)
    sw    $19, 88($26)
    sw    $20, 92($26)
    sw    $21, 96($26)
    sw    $22, 100($26)
    sw    $23, 104($26)
    sw    $24, 108($26)
    sw    $25, 112($26)

    # $26 and $27 are not saved

    sw    $28, 116($26)
    sw    $29, 120($26)
    sw    $30, 124($26)
    sw    $31, 128($26)
    mfhi  $27
    sw    $27, 132($26)
    mflo  $27
    sw    $27, 136($26)
    # all processor registers saved
    
    # gets EntryHI and stores it
    mfc0  $27, $10
    sw    $27, 0($26)
    
    # gets CAUSE register and stores it
    mfc0    $27, $13
    sw      $27, 4($26)
    
    # and now save STATUS and EPC registers
    mfc0  $27, $12
    sw    $27, 8($26)
    mfc0  $27, $14
    sw    $27, 12($26)

    # processor state saved: any register except $26 and $27 is available
    
    # BadPTE and PTEMiss handling
    
    li     $7, PTEBUGFLAG
    lw     $7, 0($7)
    
    # if $7 == 0 => no special case
    beq    $0, $7, LEndPTEMgmt
    
    #else overwrite CAUSE with correct value
    
    # load old CAUSE in $6
    lw    $6, 4($26)
    #clear it
    li    $5, CAUSENEGMASK
    and   $6, $6, $5
    
    # shift, add correct value, and re-shift CAUSE
    srl    $6, CAUSESTART 
    addu   $6, $6, $7
    sll    $6, CAUSESTART
    
    #store it back
    sw    $6, 4($26)

    # clears PTEBUGFLAG 
    li    $7, PTEBUGFLAG
    sw    $0, 0($7)
#END
    
LEndPTEMgmt:
    
    # Martin Hauser <info@martin-hauser.net>: Implanting interrupt handler for external devices
    
    # get interrupt in question
    mfc0 $4,$13
    li   $5,EXTINTMASK
    and  $4,$4,$5
    beq  $0,$4,LExtIntHandlerEnd 
    
    li   $5,EXTINTKBCHKMASK
    and  $5,$4,$5
    bgtz $5, LKBHandler
    
    li   $5,EXTINTSCRCHKMASK
    and  $5,$4,$5
    bgtz $5, LSCRHandler
    
    j LExtIntHandlerSkip
#END

LKBHandler:
    # acknowledge interrupt (even needed if cause gets overwritten later)
    mfc0 $4,$13
    li   $5,EXTINTKBDONE
    and  $4,$4,$5
    mtc0 $4,$13 
      
    li   $4,KEYBDEVID
    lw   $4,0($4)
    and  $4,0x000000FF  # get last byte only
    
    # load $5 <- KEYBBUFFSPTR
    li   $5,KEYBBUFFSPTR
    lw   $5,0($5)
    
    # check whether the place to write is sane
    
    # check whether adress is in Range
    li   $6,KEYBBUFFSTART
    add  $6,$5,$6
    li   $7,KEYBBUFFEND
    sub  $7,$7,$6
    bltz $7,LKBNotSane
    
    # check whether a zero is on the place to write
    lbu  $8,0($6)
    bne  $8,$0,LKBNotSane
    
    sb   $4,0($6)
    bne  $7,$0,LKBSkip
    
    li   $5,-1

LKBSkip:
    addi $5,$5,1
    
    # store $5 -> KEYBBUFFSPTR
    li   $4,KEYBBUFFSPTR
    sw   $5,0($4)
    
    j LExtIntHandlerEnd
#END
    
LKBNotSane:
    
    j LExtIntHandlerEnd
#END
#END    

LSCRHandler:
    # acknowledge interrupt (even needed if cause gets overwritten later)
    mfc0 $4,$13
    li   $5,EXTINTSCRDONE
    and  $4,$4,$5
    mtc0 $4,$13   

    # store 1 -> SCREENBUSYFLAG, tells print function that no restart is needed
    li   $6,SCREENBUSYFLAG
    li   $5,1
    sb   $5,0($6)

    # load $5 <- SCREENBUFFGPTR, pointer used to determine which place contains the next char
    li   $5,SCREENBUFFGPTR
    lw   $5,0($5)

    # check whether screen pointer somhow got broken and needs resetting (done in LScreenEnd)
    li   $6,SCREENBUFFSTART
    add  $6,$6,$5
    li   $7,SCREENBUFFEND
    sub  $7,$7,$6
    bltz $7, LScreenEnd  
    
    # get char into $6 and check whether loaded char is valid, reset memory address
    lbu  $4,0($6)
    sb   $0,0($6)
    blez $4,LScreenEnd
    
    # store $4 -> SCREENDEVID, writes char to screen
    li   $6,SCREENDEVID
    sw   $4,0($6)
    
    # check whether wraparound is applicable and if so, do so
    bne  $7,$0,LScreenSkip
    li   $5,-1
    
LScreenSkip:
    add  $5,$5,1
    
    # store $5 -> SCREENBUFFGPTR
    li   $6,SCREENBUFFGPTR
    sw   $5,0($6)
    
    j LExtIntHandlerEnd
#END
    

LScreenEnd:
     # store $0 -> SCREENBUFFGPTR
     li   $4,SCREENBUFFGPTR
     sw   $0,0($4)
  
     # store $0 -> SCREENBUFFSPTR
     li  $4,SCREENBUFFSPTR
     sw  $0,0($4)
  
     # store $0 -> SCREENBUSYFLAG
     li  $4,SCREENBUSYFLAG
     sb  $0,0($4)
     
     j LExtIntHandlerEnd
#END
     
     
#END

LExtIntHandlerEnd:
    
    # fix reentrance address so we'll jump back into userspace and not to 'panic'
    li   $4,VECTSIZE
    lw   $4,12($26)
    add  $4,$4,4
    sw   $4,12($26)
    mfc0 $4,$13
    sw   $4,4($26)
    
    sub $26,$26,VECTSIZE
    
#END          

LExtIntHandlerSkip:

    # end of interrupt handler implanting
    # start loading 
    addiu    $26, VECTSIZE
#END
LLoadStart:
    lw    $1, 16($26)
    lw    $2, 20($26)
    lw    $3, 24($26)
    lw    $4, 28($26)
    lw    $5, 32($26)
    lw    $6, 36($26)
    lw    $7, 40($26)
    lw    $8, 44($26)
    lw    $9, 48($26)
    lw    $10, 52($26)
    lw    $11, 56($26)
    lw    $12, 60($26)
    lw    $13, 64($26)     
    lw    $14, 68($26)
    lw    $15, 72($26)
    lw    $16, 76($26)
    lw    $17, 80($26)
    lw    $18, 84($26)
    lw    $19, 88($26)
    lw    $20, 92($26)
    lw    $21, 96($26)
    lw    $22, 100($26)
    lw    $23, 104($26)
    lw    $24, 108($26)
    lw    $25, 112($26)

    # $26 and $27 are not saved so they are not loaded too

    lw    $28, 116($26)
    lw    $29, 120($26)
    lw    $30, 124($26)
    lw    $31, 128($26)
    
    lw    $27, 132($26)
    mthi $27
    lw    $27, 136($26)
    mtlo  $27
    
    # all processor registers loaded (almost)
    
    # storing new EntryHI into CP0 register
    lw     $27, 0($26)
    mtc0   $27, $10
    
    # storing new CAUSE into CP0 register
    lw     $27, 4($26)
    mtc0   $27, $13

    # now load STATUS register
    lw    $27, 8($26)

    # this is for avoiding troubles if STATUS has (erroneously) set bit 0/1
    # it would cause an immediate EXC trap or expose BIOS to interrupts
    srl   $27, 2
    sll   $27, 2

    mtc0  $27, $12
    
    lw    $27, 12($26)

    # load new PC and jump
    .set noreorder
    .set nomacro
    jr    $27
    rfe
    .set reorder
    .set macro    
    
    .end bios
#END
#END
LbiosEnd:
    .size bios, LbiosEnd - bios
#END
