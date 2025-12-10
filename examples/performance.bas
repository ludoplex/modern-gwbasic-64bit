10 REM ===================================
20 REM Performance Test - Assembly Speed
30 REM ===================================
40 PRINT "Testing assembly-optimized operations..."
50 PRINT ""
60 REM Fast arithmetic loop
70 LET S = 0
80 FOR I = 1 TO 1000
90 S = S + I
100 NEXT I
110 PRINT "Sum 1 to 1000 = "
120 PRINT S
130 PRINT ""
140 REM Nested loops with multiplication
150 LET P = 1
160 FOR I = 1 TO 10
170 FOR J = 1 TO 10
180 P = I * J
190 NEXT J
200 NEXT I
210 PRINT "Nested loop complete"
220 PRINT ""
230 REM Function calls in loop
240 FOR I = 1 TO 5
250 PRINT SIN(I)
260 NEXT I
270 PRINT ""
280 PRINT "Performance test complete!"
290 END
