add_executable(test_core ${CMAKE_SOURCE_DIR}/core/AppControllerTest.cpp
                         ${COMPONENTS_DIR}/core/src/AppController.cpp)

target_include_directories(
  test_core
  PRIVATE ${CMAKE_SOURCE_DIR}/stubs ${COMPONENTS_DIR}/core/include
          ${COMPONENTS_DIR}/common/include ${COMPONENTS_DIR}/core/mock)

target_link_libraries(test_core GTest::GTest GTest::Main GTest::gmock
                      GTest::gmock_main)

gtest_discover_tests(test_core)
