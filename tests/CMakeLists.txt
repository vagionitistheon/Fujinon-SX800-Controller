# ==============================================================================
# Automated Unit Tests
# ==============================================================================
enable_testing()

function(add_sx800_test test_name test_source)
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} PRIVATE FujinonSX800Core)
    apply_compiler_flags(${test_name})
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

add_sx800_test(TestPelcoDFrame TestPelcoDFrame.cpp)
add_sx800_test(TestProtocolBuilder TestProtocolBuilder.cpp)
add_sx800_test(TestProtocolParser TestProtocolParser.cpp)
add_sx800_test(TestOpticalTables TestOpticalTables.cpp)
add_sx800_test(TestMockCamera TestMockCamera.cpp)
add_sx800_test(TestCircularByteRing TestCircularByteRing.cpp)
