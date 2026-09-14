# ==============================================================================
# Automated Unit Tests
# ==============================================================================
enable_testing()

function(add_sx800_test test_name test_source)
    add_executable(${test_name} ${test_source})
    target_include_directories(${test_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(${test_name} PRIVATE FujinonSX800Core)
    apply_compiler_flags(${test_name})
    if(MSVC)
        target_compile_options(${test_name} PRIVATE /UNDEBUG)
    else()
        target_compile_options(${test_name} PRIVATE -UNDEBUG)
    endif()
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

add_sx800_test(TestPelcoDFrame TestPelcoDFrame.cpp)
add_sx800_test(TestProtocolBuilder TestProtocolBuilder.cpp)
add_sx800_test(TestProtocolParser TestProtocolParser.cpp)
add_sx800_test(TestOpticalTables TestOpticalTables.cpp)
add_sx800_test(TestMockCamera TestMockCamera.cpp)
add_sx800_test(TestCircularByteRing TestCircularByteRing.cpp)
add_sx800_test(TestTransportState TestTransportState.cpp)
add_sx800_test(TestUdpTransport TestUdpTransport.cpp)
add_sx800_test(TestBusScanner TestBusScanner.cpp)
add_sx800_test(TestTransportLifecycle TestTransportLifecycle.cpp)

add_executable(TestQFujinonCamera TestQFujinonCamera.cpp)
target_include_directories(TestQFujinonCamera PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/libs/FujinonSX800Qt
)
target_link_libraries(TestQFujinonCamera PRIVATE FujinonSX800Qt Qt6::Core Qt6::Test)
apply_compiler_flags(TestQFujinonCamera)
if(MSVC)
    target_compile_options(TestQFujinonCamera PRIVATE /UNDEBUG)
else()
    target_compile_options(TestQFujinonCamera PRIVATE -UNDEBUG)
endif()
add_test(NAME TestQFujinonCamera COMMAND TestQFujinonCamera)

add_executable(TestConnectionWidget
    TestConnectionWidget.cpp
    ${CMAKE_SOURCE_DIR}/app-qt/widgets/ConnectionWidget.cpp
)
target_include_directories(TestConnectionWidget PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/app-qt
    ${CMAKE_SOURCE_DIR}/app-qt/widgets
    ${CMAKE_SOURCE_DIR}/libs/FujinonSX800Core
)
target_link_libraries(TestConnectionWidget PRIVATE FujinonSX800Core Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test)
apply_compiler_flags(TestConnectionWidget)
if(MSVC)
    target_compile_options(TestConnectionWidget PRIVATE /UNDEBUG)
else()
    target_compile_options(TestConnectionWidget PRIVATE -UNDEBUG)
endif()
add_test(NAME TestConnectionWidget COMMAND TestConnectionWidget)
set_tests_properties(TestConnectionWidget PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
