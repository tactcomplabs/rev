add_rev_test(fenv_TONEAREST  fenv 600 "all;memh;rv64;c++" SCRIPT "run_fenv_TONEAREST.sh")
add_rev_test(fenv_DOWNWARD   fenv 600 "all;memh;rv64;c++" SCRIPT "run_fenv_DOWNWARD.sh")
add_rev_test(fenv_UPWARD     fenv 600 "all;memh;rv64;c++" SCRIPT "run_fenv_UPWARD.sh")
add_rev_test(fenv_TOWARDZERO fenv 600 "all;memh;rv64;c++" SCRIPT "run_fenv_TOWARDZERO.sh")
