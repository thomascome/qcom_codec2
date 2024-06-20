project(codec2_test)

set(CODEC2_TEST_NAME codec2_test)

add_executable(${CODEC2_TEST_NAME}
    codec2_test.cc
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

target_link_libraries(${CODEC2_TEST_NAME} base)
target_link_libraries(${CODEC2_TEST_NAME} qcom_codec2)

install(TARGETS ${CODEC2_TEST_NAME} RUNTIME DESTINATION "bin")