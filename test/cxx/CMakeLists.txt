# RevCPU test/cxx/CMakeLists.txt
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

add_rev_test(SIMPLE_STRUCT simple_struct 30 "test_level=2;rv64;memh;c++")
add_subdirectory(stl)
