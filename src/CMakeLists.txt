
include_directories(flags)


add_library(base STATIC "")
target_sources(base PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/base/event_wrapper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/base/logging.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/base/utils.cc
)
target_include_directories(base PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/base)


add_library(net STATIC "")
target_sources(net PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/net/http_session.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/net/server.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/net/socket_wrapper.cpp
)
target_include_directories(net PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/net)
target_link_libraries(net base)


add_executable(server ${CMAKE_CURRENT_SOURCE_DIR}/main.cc)
target_link_libraries(server net)


if (ENABLE_SANITIZER)
    target_link_libraries(my_server asan ubsan)
endif()
