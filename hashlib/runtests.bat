@set testcount=0
make build
@if errorlevel 1 goto failure

@echo.
hashtest 1 >junk
@if errorlevel 1 goto failure
diff -q junk test1.txt
@if errorlevel 1 goto failure
@set testcount=1

@echo.
hashtest 2 >junk
@if errorlevel 1 goto failure
diff -q junk test2.txt
@if errorlevel 1 goto failure
@set testcount=2

@echo.
hashtest 3 >junk
@if errorlevel 1 goto failure
:: This test is only valid when hashlib cannot
:: reorganize to expand the hashtable size
:: force reorganize to return 0 to enable this test
:: -------- 1st option for alterable test --------
::diff -q junk test3a.txt
:: -------- 2nd option for reorganize on ---------
diff -q junk test3.txt
:: ------------ end of alterable test ------------

@if errorlevel 1 goto failure
@set testcount=3

@echo.
hashtest 4 >junk
@if errorlevel 1 goto failure
diff -q junk test4.txt
@if errorlevel 1 goto failure
@set testcount=4

@echo.
hashtest 4 100001 >junk
@if errorlevel 1 goto failure
diff -q junk test4a.txt
@if errorlevel 1 goto failure
@set testcount=5

@echo.
hashtest 4 400001 >junk
@if errorlevel 1 goto failure
diff -q junk test4b.txt
@if errorlevel 1 goto failure
@set testcount=6

@echo.
hashtest 5 >junk
@if errorlevel 1 goto failure
diff -q junk test5.txt
@if errorlevel 1 goto failure
@set testcount=7

@echo.
hashtest 6 >junk
@if errorlevel 1 goto failure
diff -q junk test6.txt
@if errorlevel 1 goto failure
@set testcount=8

@echo.
hashtest 7 >junk
@if errorlevel 1 goto failure
diff -q junk test7.txt
@if errorlevel 1 goto failure
@set testcount=9

@echo.
markov gpl.txt >junk
@if errorlevel 1 goto failure
diff -q junk markov.txt
@if errorlevel 1 goto failure
@set testcount=10

@goto end

:failure
@echo Test failed

:end
@echo %testcount% tests passed

