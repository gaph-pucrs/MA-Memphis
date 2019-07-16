vsim -novopt -t 10ns +notimingchecks hardware/work.test_bench

do wave.do
onerror {resume}
radix hex
set NumericStdNoWarnings 1
set StdArithNoWarnings 1

