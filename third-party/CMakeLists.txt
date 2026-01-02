if (APPLE OR build-dependencies)
#    file(COPY "${CMAKE_SOURCE_DIR}/third-party/usr/lib/libduktape.so" DESTINATION "${CMAKE_BINARY_DIR}/")
#    file(COPY "${CMAKE_SOURCE_DIR}/third-party/usr/lib/libduktape.207.so" DESTINATION "${CMAKE_BINARY_DIR}/")
#    file(COPY "${CMAKE_SOURCE_DIR}/third-party/usr/lib/libduktape.207.20700.so" DESTINATION "${CMAKE_BINARY_DIR}/")

    if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
        cmake_policy(SET CMP0135 NEW)
    endif()

    include_directories(${CMAKE_BINARY_DIR}/ ${CMAKE_BINARY_DIR}/usr/include/ ${CMAKE_BINARY_DIR}/usr/include/bson-${MONGOC_VERSION} /usr/include/ lib/ lib/acceleration/)
    list(APPEND CMAKE_REQUIRED_INCLUDES
            ${CMAKE_BINARY_DIR}/usr/include/
            ${CMAKE_BINARY_DIR}/third-party
            ${CMAKE_BINARY_DIR}/third-party/usr/include/
            /opt/homebrew/include/
    )

    LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/third-party/usr/lib ${CMAKE_BINARY_DIR}/usr/lib /usr/local/lib/ /usr/lib64/ /usr/local/lib/ lib/acceleration/)

    include(ExternalProject)

    function(move_file file_name_to_move)
        file(COPY "${file_name_to_move}" DESTINATION "${CMAKE_BINARY_DIR}/")
    endfunction()

    move_file(${CMAKE_SOURCE_DIR}/third-party/cmph-2.0.2.tar.gz)
    move_file(${CMAKE_SOURCE_DIR}/third-party/pcre-8.45.tar.gz)
    move_file(${CMAKE_SOURCE_DIR}/third-party/duktape-2.7.0.tar.xz)
    move_file(${CMAKE_SOURCE_DIR}/third-party/discodb-0.9.5.tar.gz)
    move_file(${CMAKE_SOURCE_DIR}/third-party/libsodium-1.0.20.tar.gz)
    move_file(${CMAKE_SOURCE_DIR}/third-party/mongo-c-driver-${MONGOC_VERSION}.tar.gz)
    file(COPY "${CMAKE_SOURCE_DIR}/third-party/uthash.h" DESTINATION "${CMAKE_BINARY_DIR}/usr/include")
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/third-party/discodb_build")


    ExternalProject_Add(
            ExternalSodium # Name for the external project
            URL "file:///${CMAKE_SOURCE_DIR}/third-party/libsodium-1.0.20.tar.gz" # Path to your tarball
            URL_HASH "SHA256=ebb65ef6ca439333c2bb41a0c1990587288da07f6c7fd07cb3a18cc18d30ce19"
            SOURCE_DIR "${CMAKE_BINARY_DIR}/third-party/sodium_src"
            BINARY_DIR "${CMAKE_BINARY_DIR}/third-party/sodium_src"
            INSTALL_DIR "${CMAKE_BINARY_DIR}/usr"
            CONFIGURE_COMMAND ./configure --prefix=${CMAKE_BINARY_DIR}/usr
            BUILD_COMMAND make
            INSTALL_COMMAND make install
    )

    ExternalProject_Add(
            ExternalPCRE # Name for the external project
            URL "file:///${CMAKE_SOURCE_DIR}/third-party/pcre-8.45.tar.gz" # Path to your tarball
            URL_HASH "SHA256=4e6ce03e0336e8b4a3d6c2b70b1c5e18590a5673a98186da90d4f33c23defc09"
            SOURCE_DIR "${CMAKE_BINARY_DIR}/third-party/pcre_src"
            BINARY_DIR "${CMAKE_BINARY_DIR}/third-party/pcre_src"
            INSTALL_DIR "${CMAKE_BINARY_DIR}/usr"
            CONFIGURE_COMMAND ./configure --prefix=${CMAKE_BINARY_DIR}/usr --enable-utf --enable-jit=auto --enable-pcre32 --enable-pcre16 --enable-pcre8 --disable-cpp --enable-newline-is-anycrlf --with-match-limit=100;
            BUILD_COMMAND make
            INSTALL_COMMAND make install
            DEPENDS ExternalSodium
    )


    ExternalProject_Add(
            ExternalDuktape # Name for the external project
            URL "file:///${CMAKE_SOURCE_DIR}/third-party/duktape-2.7.0.tar.xz" # Path to your tarball
            URL_HASH "SHA256=90f8d2fa8b5567c6899830ddef2c03f3c27960b11aca222fa17aa7ac613c2890"
            SOURCE_DIR "${CMAKE_BINARY_DIR}/third-party/duk_src"
            BINARY_DIR "${CMAKE_BINARY_DIR}/third-party/duk_src"
            INSTALL_DIR "${CMAKE_BINARY_DIR}/usr"
            CONFIGURE_COMMAND echo ok
            BUILD_COMMAND make -f Makefile.sharedlibrary INSTALL_PREFIX=${CMAKE_BINARY_DIR}/usr
            INSTALL_COMMAND make -f Makefile.sharedlibrary INSTALL_PREFIX=${CMAKE_BINARY_DIR}/usr install
            BUILD_BYPRODUCTS "${CMAKE_BINARY_DIR}/third-party/usr/libduktape.so"
            DEPENDS ExternalSodium
    )
    add_custom_target(ExternalDuktapeDebuggerFix
            COMMAND  cp usr/lib/libduktape*.so .;
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Shared Object Move so things work"
            DEPENDS ExternalDuktape
    )

    ExternalProject_Add(
            ExternalCMPH # Name for the external project
            URL "file:///${CMAKE_SOURCE_DIR}/third-party/cmph-2.0.2.tar.gz" # Path to your tarball
            URL_HASH "SHA256=365f1e8056400d460f1ee7bfafdbf37d5ee6c78e8f4723bf4b3c081c89733f1e"
            SOURCE_DIR "${CMAKE_BINARY_DIR}/third-party/cmph_src"
            BINARY_DIR "${CMAKE_BINARY_DIR}/third-party/cmph_src"
            INSTALL_DIR "${CMAKE_BINARY_DIR}/usr"
            CONFIGURE_COMMAND autoreconf -f -i && ./configure --prefix=${CMAKE_BINARY_DIR}/usr --with-pic --with-spoon=SPOON;
            BUILD_COMMAND make
            INSTALL_COMMAND make install
    )
    add_custom_target(ExternalCMPHDebuggerFix
            COMMAND  cp usr/lib/libcmph* .
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Shared Object Move so things work"
            DEPENDS ExternalCMPH
    )
    ExternalProject_Add(
            discodbcompile # Name for the external project
            URL "file:///${CMAKE_SOURCE_DIR}/third-party/discodb-0.9.5.tar.gz" # Path to your tarball
            URL_HASH "SHA256=b9c89b5206e2183292bbd7df58ceb5e54db48d18c71c3611bd1cf18cbdbf0457"
            SOURCE_DIR "${CMAKE_BINARY_DIR}/third-party/discodb_src"
            BINARY_DIR "${CMAKE_BINARY_DIR}/third-party/discodb_build"
            INSTALL_DIR "${CMAKE_BINARY_DIR}/usr"
            CONFIGURE_HANDLED_BY_BUILD  true
            CMAKE_ARGS
                -DCMAKE_VERBOSE_MAKEFILE=ON
                -DCMAKE_BUILD_TYPE=Release
                -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                -DCMAKE_LIBRARY_PATH=${CMAKE_SOURCE_DIR}/usr/lib
                -DCMAKE_INCLUDE_PATH=${CMAKE_BINARY_DIR}/usr/include
                -DCMAKE_REQUIRED_INCLUDES=${CMAKE_BINARY_DIR}/usr/include
                -DCMAKE_PROJECT_libdiscodb_INCLUDE=${CMAKE_CURRENT_SOURCE_DIR}/third-party.cmake
                -DINCLUDE_DIRECTORIES:STRING=${CMAKE_SOURCE_DIR}/usr/include
            PATCH_COMMAND patch -p1 -d ${CMAKE_BINARY_DIR}/third-party/discodb_src < ${CMAKE_SOURCE_DIR}/third-party/discodb-0.9.5.patch
            BUILD_COMMAND ${CMAKE_COMMAND} --build .
            INSTALL_COMMAND ${CMAKE_COMMAND} --install .
            DEPENDS ExternalCMPHDebuggerFix
    )
    add_custom_target(ExternaldiscodbcompileFix
            COMMAND  cp usr/lib/libdisco* .
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Shared Object Move so things work"
            DEPENDS discodbcompile
    )
    ExternalProject_Add(
            mongoccompile # Name for the external project
            URL "file:///${CMAKE_SOURCE_DIR}/third-party/mongo-c-driver-${MONGOC_VERSION}.tar.gz" # Path to your tarball
            URL_HASH "SHA256=bc56f9acd91454eca5121e030fc1bb236283a94f4522b749beeb55e5e30343fb"
            SOURCE_DIR "${CMAKE_BINARY_DIR}/third-party/mongoc_src"
            BINARY_DIR "${CMAKE_BINARY_DIR}/third-party/mongoc_build"
            INSTALL_DIR "${CMAKE_BINARY_DIR}/usr"
            CMAKE_ARGS
            -DCMAKE_VERBOSE_MAKEFILE=ON
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            -DCMAKE_INSTALL_LIBDIR=lib
            -DCMAKE_BUILD_TYPE=Debug
            -DENABLE_MONGODB_AWS_AUTH=OFF
            -DENABLE_CLIENT_SIDE_ENCRYPTION=OFF
            -DCMAKE_INCLUDE_PATH=${CMAKE_BINARY_DIR}/usr/include
            -DCMAKE_REQUIRED_INCLUDES=${CMAKE_BINARY_DIR}/usr/include
            -DCMAKE_PROJECT_mongo-c-driver_INCLUDE=${CMAKE_CURRENT_SOURCE_DIR}/third-party.cmake
            BUILD_COMMAND ${CMAKE_COMMAND} --build .
            INSTALL_COMMAND ${CMAKE_COMMAND} --install .
            DEPENDS discodbcompile
    )


    install(DIRECTORY "${CMAKE_BINARY_DIR}/usr/lib/"
            DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
    install(DIRECTORY "${CMAKE_BINARY_DIR}/usr/include/"
            DESTINATION "${CMAKE_INSTALL_PREFIX}/include")
    install(DIRECTORY "${CMAKE_BINARY_DIR}/usr/bin/"
            DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")



else ()

endif ()