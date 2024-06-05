project(qcomm_codec2)

set(QCOMM_ENCODER_NAME "qcomm_codec2")

add_definitions(-D_LINUX_)

add_library(${QCOMM_ENCODER_NAME} SHARED
    c2-module.cc
)

set_target_properties(${QCOMM_ENCODER_NAME} PROPERTIES PUBLIC_HEADER
  c2-module.h
)

target_link_libraries(${QCOMM_ENCODER_NAME} PRIVATE codec2_vndk)

install(TARGETS ${QCOMM_ENCODER_NAME}
        LIBRARY DESTINATION "lib"
        PUBLIC_HEADER DESTINATION "include")