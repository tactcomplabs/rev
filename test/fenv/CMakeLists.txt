add_rev_test(fenv_TONEAREST  . 1000 "all;memh;rv64;c++" SCRIPT "run_fenv_TONEAREST.sh")
add_rev_test(fenv_DOWNWARD   . 1000 "all;memh;rv64;c++" SCRIPT "run_fenv_DOWNWARD.sh")
add_rev_test(fenv_UPWARD     . 1000 "all;memh;rv64;c++" SCRIPT "run_fenv_UPWARD.sh")
add_rev_test(fenv_TOWARDZERO . 1000 "all;memh;rv64;c++" SCRIPT "run_fenv_TOWARDZERO.sh")
