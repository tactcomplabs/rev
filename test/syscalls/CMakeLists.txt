add_rev_test(CHDIR chdir 30 "all;rv64;syscalls;memh")
add_rev_test(WRITE write 30 "all;rv64;syscalls;memh")
add_rev_test(FILE_IO file_io 30 "all;rv64;syscalls;memh")
add_rev_test(GETCWD getcwd 30 "all;rv64;syscalls;memh")
add_rev_test(MUNMAP munmap 30 "all;rv64;syscalls;memh")
add_rev_test(PERF_STATS perf_stats 30 "all;rv64;syscalls;memh")
# TODO: Merge this PR then merge the sbrk fix then re-enable this test
# add_rev_test(VECTOR vector 30 "all;rv64;syscalls")
