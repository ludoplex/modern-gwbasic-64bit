10 REM Test file I/O
20 OPEN "test.txt" FOR OUTPUT AS #1
30 PRINT #1, "Hello from GW-BASIC"
40 PRINT #1, "Line 2"
50 CLOSE #1
60 PRINT "File written successfully!"
70 END
