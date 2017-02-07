ADD_EXECUTABLE(bulb_controller app.c awa.c colors.c)
ADD_SUBDIRECTORY(libgatt)
FIND_LIBRARY(LIB_AWA libawa.so ${STAGING_DIR}/usr/lib)
FIND_LIBRARY(LIB_GLIB libglib-2.0.so ${STAGING_DIR}/usr/lib)
TARGET_LINK_LIBRARIES(bulb_controller ${LIB_AWA} ${LIB_GLIB} libgatt)

	
INSTALL(TARGETS bulb_controller RUNTIME DESTINATION bin)