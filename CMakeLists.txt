set(HEADERS
    include/logger/Logger.hpp
    src/logger/Logger.cpp
)

add_library(cognitiv_logger
	${HEADERS}
)

target_include_directories(cognitiv_logger
    PUBLIC
        include/
    PRIVATE
        src/
)

target_link_libraries(cognitiv_logger INTERFACE
    spdlog
)

add_library(cognitiv::logger ALIAS cognitiv_logger)
