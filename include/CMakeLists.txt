set(HEADERS
        noncopyable.hpp
        TimeStamp.hpp
        Logger.hpp
)

add_library(mudong-ev STATIC ${HEADERS})
set_target_properties(mudong-ev PROPERTIES LINKER_LANGUAGE CXX)

install(TARGETS mudong-ev DESTINATION lib)

install(FILES ${HEADERS} DESTINATION include)