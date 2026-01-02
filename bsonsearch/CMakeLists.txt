
find_package(Python COMPONENTS Interpreter Development REQUIRED)

file(COPY "${CMAKE_SOURCE_DIR}/bsonsearch" DESTINATION "${CMAKE_BINARY_DIR}/")
file(COPY "${CMAKE_SOURCE_DIR}/pyproject.toml" DESTINATION "${CMAKE_BINARY_DIR}/")
file(COPY "${CMAKE_SOURCE_DIR}/setup.py" DESTINATION "${CMAKE_BINARY_DIR}/")

add_custom_target(pybsonsearch
        ALL
        COMMAND LIB_DIR=${CMAKE_BINARY_DIR}:${CMAKE_BINARY_DIR}/usr/lib/ INCLUDE_DIR=${CMAKE_BINARY_DIR}:${CMAKE_SOURCE_DIR}/lib/:${CMAKE_BINARY_DIR}/usr/include/:${CMAKE_BINARY_DIR}/usr/include/bson-${MONGOC_VERSION} ${Python_EXECUTABLE} -m build;
        COMMAND rm -rf bsonsearch
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Making the python library via cmake is a confusing"
        DEPENDS bsonsearch
)
