FIND_LIBRARY(EMBREE_LIBRARIES NAMES libembree3.a embree3
    $ENV{EMBREE_PATH}
    $ENV{EMBREE_PATH}/lib/
    ${PROJECT_SOURCE_DIR}/python/pymesh/third_party/lib/
    /opt/local/lib/
    /usr/local/lib/
    /usr/lib/)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Embree
    "Embree library cannot be found.  Consider set EMBREE_PATH environment variable"
    EMBREE_LIBRARIES)

MESSAGE(INFO "Embree library: ${EMBREE_LIBRARIES}")

MARK_AS_ADVANCED(
    EMBREE_LIBRARIES)

