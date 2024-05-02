# RevCPU test/cxx/stl CMakeLists.txt
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

#add_rev_test(array array 30 "all;memh;rv64;c++" SCRIPT "run_array.sh")
add_rev_test(map map 30 "all;memh;rv64;c++" SCRIPT "run_map.sh")
add_rev_test(map_obj map_obj 30 "all;memh;rv64;c++" SCRIPT "run_map_obj.sh")
add_rev_test(multimap multimap 30 "all;memh;rv64;c++" SCRIPT "run_multimap.sh")
add_rev_test(multimap_obj multimap_obj 30 "all;memh;rv64;c++" SCRIPT "run_multimap_obj.sh")
add_rev_test(vector vector 30 "all;memh;rv64;c++" SCRIPT "run_vector.sh")
#add_rev_test(vector_edge vector_edge 30 "all;memh;rv64;c++" SCRIPT "run_vector_edge.sh")
#add_rev_test(vector_int64_t vector_int64_t 30 "all;memh;rv64;c++" SCRIPT "run_vector_int64_t.sh")
add_rev_test(vector_obj vector_obj 30 "all;memh;rv64;c++" SCRIPT "run_vector_obj.sh")
add_rev_test(vector_pair vector_pair 30 "all;memh;rv64;c++" SCRIPT "run_vector_pair.sh")
#add_rev_test(vector_vector_int64_t vector_vector_int64_t 30 "all;memh;rv64;c++" SCRIPT "run_vector_vector_int64_t.sh")
add_rev_test(unordered_map unordered_map 30 "all;memh;rv64;c++" SCRIPT "run_unordered_map.sh")
add_rev_test(unordered_map_obj unordered_map_obj 30 "all;memh;rv64;c++" SCRIPT "run_unordered_map_obj.sh")

