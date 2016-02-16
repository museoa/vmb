	.section    .text,"ax",@progbits
	.global DTrapTable
	.global FTrapTable


	% The tables go into a separate file which is assembled
	% with the -no-expand option to make shure the table keeps its structure
	
DTrapTable JMP DTrapUnhandled  %0
	   JMP DTrapUnhandled  %1
           JMP DTrapUnhandled  %2
           JMP DTrapUnhandled  %3
           JMP DTrapUnhandled  %4
           JMP DTrapUnhandled  %5
           JMP DTrapUnhandled  %6
           JMP DTrapUnhandled  %7
           JMP DTrapUnhandled  %8
           JMP DTrapUnhandled  %9
           JMP DTrapUnhandled  %10
           JMP DTrapUnhandled  %11
           JMP DTrapUnhandled  %12
           JMP DTrapUnhandled  %13
           JMP DTrapUnhandled  %14
           JMP DTrapUnhandled  %15
           JMP DTrapUnhandled  %16
           JMP DTrapKey        %17
           JMP DTrapScreen     %18
           JMP DTrapDisk       %19
           JMP DTrapUnhandled  %20
           JMP DTrapUnhandled  %21
           JMP DTrapUnhandled  %22
           JMP DTrapUnhandled  %23
           JMP DTrapUnhandled  %24
           JMP DTrapUnhandled  %25
           JMP DTrapUnhandled  %26
           JMP DTrapUnhandled  %27
           JMP DTrapUnhandled  %28
           JMP DTrapUnhandled  %29
           JMP DTrapUnhandled  %30
           JMP DTrapUnhandled  %31
           JMP DTrapUnhandled  %32
           JMP DTrapUnhandled  %33
           JMP DTrapUnhandled  %34
           JMP DTrapUnhandled  %35
           JMP DTrapUnhandled  %36
           JMP DTrapUnhandled  %37
           JMP DTrapUnhandled  %38
           JMP DTrapUnhandled  %39
           JMP DTrapUnhandled  %40
           JMP DTrapUnhandled  %41
           JMP DTrapUnhandled  %42
           JMP DTrapUnhandled  %43
           JMP DTrapUnhandled  %44
           JMP DTrapUnhandled  %45
           JMP DTrapUnhandled  %46
           JMP DTrapUnhandled  %47
           JMP DTrapUnhandled  %48
           JMP DTrapUnhandled  %49
           JMP DTrapUnhandled  %50
           JMP DTrapUnhandled  %51
           JMP DTrapUnhandled  %52
           JMP DTrapUnhandled  %53
           JMP DTrapUnhandled  %54
           JMP DTrapUnhandled  %55
           JMP DTrapUnhandled  %56
           JMP DTrapUnhandled  %57
           JMP DTrapUnhandled  %58
           JMP DTrapUnhandled  %59
           JMP DTrapUnhandled  %60
           JMP DTrapUnhandled  %61
           JMP DTrapUnhandled  %62
           JMP DTrapUnhandled  %63
           JMP DTrapUnhandled  %64  rQ was zero



FTrapTable JMP   TrapHalt       %0
	   JMP   TrapFopen      %1
	   JMP   TrapFclose     %2
	   JMP   TrapFread      %3
	   JMP   TrapFgets      %4
	   JMP   TrapFgetws     %5
	   JMP   TrapFwrite     %6
	   JMP   TrapFputs      %7 
	   JMP   TrapFputws     %8
	   JMP   TrapFseek      %9
	   JMP   TrapFtell      %a
	   JMP   TrapUnhandled  %b
	   JMP   TrapUnhandled  %c
	   JMP   TrapUnhandled  %d
	   JMP   TrapUnhandled  %e
	   JMP   TrapUnhandled  %f
	   JMP   TrapGPutPixel %10
	   JMP   TrapUnhandled %11
	   JMP   TrapUnhandled %12
	   JMP   TrapUnhandled %13
	   JMP   TrapUnhandled %14
	   JMP   TrapUnhandled %15
	   JMP   TrapUnhandled %16
	   JMP   TrapUnhandled %17
	   JMP   TrapUnhandled %18
	   JMP   TrapUnhandled %19
	   JMP   TrapUnhandled %1a
	   JMP   TrapUnhandled %1b
	   JMP   TrapUnhandled %1c
	   JMP   TrapUnhandled %1d
	   JMP   TrapUnhandled %1e
	   JMP   TrapUnhandled %1f
