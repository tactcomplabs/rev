add_rev_test(fenv_TONEAREST  . 1000 "all;memh;rv64;c++" SCRIPT "run_TONEAREST.sh")
add_rev_test(fenv_DOWNWARD   . 1000 "all;memh;rv64;c++" SCRIPT "run_DOWNWARD.sh")
add_rev_test(fenv_UPWARD     . 1000 "all;memh;rv64;c++" SCRIPT "run_UPWARD.sh")
add_rev_test(fenv_TOWARDZERO . 1000 "all;memh;rv64;c++" SCRIPT "run_TOWARDZERO.sh")
