
GPutPixel	IS #10

		LOC #100
Main		SET $0,100      x
		SET $1,200      y
		SET $2,150      counter
		SET $3,640      length of one screen line
		MUL $3,$3,$1    offset for line y
		ADD $3,$3,$0    offset for point (x,y)
                SET $4,#FF      color blue
1H		SL  $255,$3,34  offset*4 parameter
		OR  $255,$255,$4 color paramter
		TRAP 0,GPutPixel,0
		ADD $3,$3,1
		SUB $2,$2,1
		BP  $2,1B
		TRAP 0,Halt,0
		
                
	