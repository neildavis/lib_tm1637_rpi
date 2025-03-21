# these are cache variables, so they could be overwritten with -D,
set(CPACK_PACKAGE_NAME "lib${PROJECT_NAME}"
    CACHE STRING "The resulting package name"
)
# which is useful in case of packing only selected components instead of the whole thing
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION}
    CACHE STRING "Package description for the package metadata"
)
set(CPACK_PACKAGE_VENDOR "Neil Davis")

set(CPACK_VERBATIM_VARIABLES YES)

set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
SET(CPACK_OUTPUT_FILE_PREFIX "${CMAKE_SOURCE_DIR}/_packages")


set(CPACK_STRIP_FILES YES)

set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local")

set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

set(CPACK_PACKAGE_CONTACT "3934226+neildavis@users.noreply.github.com")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Neil Davis <${CPACK_PACKAGE_CONTACT}>")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA 
    "${CMAKE_CURRENT_SOURCE_DIR}/scripts/postinst;${CMAKE_CURRENT_SOURCE_DIR}/scripts/postrm")

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

# package name for deb. If set, then instead of some-application-0.9.2-Linux.deb
# you'll get some-application_0.9.2_amd64.deb (note the underscores too)
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

include(CPack)