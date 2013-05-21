# kQOAuth library search

IF (QTKOAUTH_INCLUDE_DIR AND QTKOAUTH_LIBRARIES)
	# check cache
	SET(QTKOAUTH_FOUND TRUE)
ELSE (QTKOAUTH_INCLUDE_DIR AND QTKOAUTH_LIBRARIES)
	IF (NOT WIN32)
		FIND_PACKAGE(PkgConfig) 
		PKG_CHECK_MODULES(PC_QTKOAUTH QUIET kqoauth)
		SET(QTKOAUTH_DEFINITIONS ${PC_QTKOAUTH_CFLAGS_OTHER})

		find_path(QTKOAUTH_INCLUDE_DIR QtKOAuth/kqoauthrequest.h
			HINTS ${PC_QTKOAUTH_INCLUDEDIR} ${PC_QTKOAUTH_INCLUDE_DIRS}
			PATH_SUFFIXES QtKOAuth )

		find_library(QTKOAUTH_LIBRARY NAMES kqoauth
			HINTS ${PC_QTKOAUTH_LIBDIR} ${PC_QTKOAUTH_LIBRARY_DIRS} )

		set(QTKOAUTH_LIBRARIES ${QTKOAUTH_LIBRARY} )
		set(QTKOAUTH_INCLUDE_DIRS ${QTKOAUTH_INCLUDE_DIR} )
	ELSE (NOT WIN32)
		IF (NOT DEFINED QTKOAUTH_DIR)
			IF (QTKOAUTH_FIND_REQUIRED)
				MESSAGE(FATAL_ERROR "Please set QTKOAUTH_DIR variable")
			ELSE (QTKOAUTH_FIND_REQUIRED)
				MESSAGE(STATUS "Please set QTKOAUTH_DIR variable for OAuth login support")
			ENDIF (QTKOAUTH_FIND_REQUIRED)
		ENDIF (NOT DEFINED QTKOAUTH_DIR)

		SET (QTKOAUTH_INCLUDE_WIN32 ${QTKOAUTH_DIR})      

		SET (PROBE_DIR_Debug
			${QTKOAUTH_DIR}/build/lib/Debug)
		SET (PROBE_DIR_Release
			${QTKOAUTH_DIR}/build/lib/MinSizeRel ${QTKOAUTH_DIR}/build/lib/Release)

		FIND_LIBRARY (QTKOAUTH_LIBRARY_DEBUG NAMES kqoauth.lib PATHS ${PROBE_DIR_Debug})
		FIND_LIBRARY (QTKOAUTH_LIBRARY_RELEASE NAMES kqoauth.lib PATHS ${PROBE_DIR_Release})
		win32_tune_libs_names (QTKOAUTH)   
	ENDIF (NOT WIN32)

	IF (NOT WIN32) # regression guard
		FIND_PATH (QTKOAUTH_INCLUDE_DIR
			NAMES
				QtKOAuth/kqoauthrequest.h
			PATH_SUFFIXES
				QtKOAuth
			PATHS
				${QTKOAUTH_DIR}
				${INCLUDE_INSTALL_DIR}
				${KDE4_INCLUDE_DIR}
			)

	ELSE (NOT WIN32) #may be works for linux too
		FIND_PATH (QTKOAUTH_INCLUDE_DIR
			NAMES
				QtKOAuth/kqoauthrequest.h
			PATH_SUFFIXES
				QtKOAuth
			PATHS
				${QTKOAUTH_DIR}
				${INCLUDE_INSTALL_DIR}
				${KDE4_INCLUDE_DIR}
				${QTKOAUTH_INCLUDE_WIN32}
			)
	ENDIF (NOT WIN32)

	IF (QTKOAUTH_INCLUDE_DIR AND QTKOAUTH_LIBRARIES)
		SET (QTKOAUTH_FOUND 1)
	ENDIF (QTKOAUTH_INCLUDE_DIR AND QTKOAUTH_LIBRARIES)

	IF (QTKOAUTH_FOUND)
		MESSAGE (STATUS "Found the QTKOAUTH libraries at ${QTKOAUTH_LIBRARIES}")
		MESSAGE (STATUS "Found the QTKOAUTH headers at ${QTKOAUTH_INCLUDE_DIR}")
		IF (WIN32)
			MESSAGE (STATUS ${_WIN32_ADDITIONAL_MESS})
		ENDIF (WIN32)
	ELSE (QTKOAUTH_FOUND)
		IF (QTKOAUTH_FIND_REQUIRED)
			MESSAGE (FATAL_ERROR "Could NOT find required QTKOAUTH library, aborting")
		ELSE (QTKOAUTH_FIND_REQUIRED)
			MESSAGE (STATUS "Could NOT find QTKOAUTH")
		ENDIF (QTKOAUTH_FIND_REQUIRED)
	ENDIF (QTKOAUTH_FOUND)

ENDIF (QTKOAUTH_INCLUDE_DIR AND QTKOAUTH_LIBRARIES)
