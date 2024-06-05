project(qcomm_codec2)

set(QCOMM_ENCODER_NAME "qcomm_codec2")

add_definitions(-D_LINUX_)

add_library(${QCOMM_ENCODER_NAME} SHARED
    c2_module.cc
    c2_engine.cc
)

set_target_properties(${QCOMM_ENCODER_NAME} PROPERTIES PUBLIC_HEADER
  c2_module.h
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

target_link_libraries(${QCOMM_ENCODER_NAME} PRIVATE 
                      base
                      codec2_vndk)

install(TARGETS ${QCOMM_ENCODER_NAME}
        LIBRARY DESTINATION "lib"
        PUBLIC_HEADER DESTINATION "include")