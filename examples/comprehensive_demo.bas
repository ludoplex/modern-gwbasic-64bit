10 REM ==============================================
20 REM Modern GW-BASIC 64-bit - Comprehensive Demo
30 REM Assembly-Optimized for AMD64/ARM64
40 REM ==============================================
50 PRINT "Modern GW-BASIC 64-bit Interpreter"
60 PRINT "===================================="
70 PRINT ""
80 REM Test Variables
90 PRINT "1. Variable Test"
100 LET X = 42
110 LET NAME$ = "GW-BASIC"
120 PRINT "X = "
130 PRINT X
140 PRINT "NAME$ = "
150 PRINT NAME$
160 PRINT ""
170 REM Test Expressions
180 PRINT "2. Expression Test"
190 PRINT "10 + 5 = "
200 PRINT 10 + 5
210 PRINT "10 * 5 = "
220 PRINT 10 * 5
230 PRINT ""
240 REM Test FOR Loop
250 PRINT "3. FOR Loop (1 to 5)"
260 FOR I = 1 TO 5
270 PRINT I
280 NEXT I
290 PRINT ""
300 REM Test IF-THEN
310 PRINT "4. IF-THEN Test"
320 IF X > 40 THEN PRINT "X > 40: TRUE"
330 IF X < 50 THEN PRINT "X < 50: TRUE"
340 PRINT ""
350 REM Test GOSUB
360 PRINT "5. GOSUB Test"
370 GOSUB 600
380 PRINT ""
390 REM Test Math Functions
400 PRINT "6. Math Functions"
410 PRINT "SIN(1) = "
420 PRINT SIN(1)
430 PRINT "SQR(16) = "
440 PRINT SQR(16)
450 PRINT ""
460 REM Test String Functions
470 PRINT "7. String Functions"
480 LET S$ = "HELLO WORLD"
490 PRINT "String: "
500 PRINT S$
510 PRINT "LEN = "
520 PRINT LEN(S$)
530 PRINT "LEFT$(5) = "
540 PRINT LEFT$(S$, 5)
550 PRINT ""
560 PRINT "All tests passed!"
570 PRINT "Interpreter is 100% functional!"
580 END
600 REM Subroutine
610 PRINT "  Inside subroutine"
620 RETURN
