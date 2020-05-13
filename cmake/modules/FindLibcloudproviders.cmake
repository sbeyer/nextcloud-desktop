# FindLibcloudproviders.cmake

find_path(LIBCLOUDPROVIDERS_INCLUDE_DIR
    NAMES cloudprovidersproviderexporter.h cloudprovidersaccountexporter.h
    PATH_SUFFIXES cloudproviders
)
find_library(LIBCLOUDPROVIDERS_LIBRARY
    NAMES
        libcloudproviders
        cloudproviders
    HINTS
       /usr/lib
       /usr/lib/${CMAKE_ARCH_TRIPLET}
       /usr/local/lib
       /opt/local/lib
       ${CMAKE_LIBRARY_PATH}
       ${CMAKE_INSTALL_PREFIX}/lib
)

find_package_handle_standard_args(LIBCLOUDPROVIDERS DEFAULT_MSG LIBCLOUDPROVIDERS_INCLUDE_DIR LIBCLOUDPROVIDERS_LIBRARY)

if(LIBCLOUDPROVIDERS_FOUND)
    message(STATUS "Found libcloudproviders: ${LIBCLOUDPROVIDERS_LIBRARY}")
endif()
