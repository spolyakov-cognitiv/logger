set(HEADERS
    Logger.cpp
    Logger.hpp
)

add_library(logger INTERFACE
	${HEADERS}
)

target_link_libraries(logger INTERFACE
    spdlog
)
