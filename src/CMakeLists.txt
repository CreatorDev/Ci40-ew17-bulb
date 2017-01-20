
ADD_EXECUTABLE(bulb_controller bulb_controller.c)

FIND_LIBRARY(LIB_AWA libawa.so ${STAGING_DIR}/usr/lib)
TARGET_LINK_LIBRARIES(bulb_controller ${LIB_AWA})
	
INSTALL(TARGETS bulb_controller RUNTIME DESTINATION bin)