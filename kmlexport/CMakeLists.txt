
SET(kipiplugin_kmlexport_PART_SRCS plugin_kmlexport.cpp
                                   gpsdataparser.cpp
                                   kmlexport.cpp
                                   kmlgpsdataparser.cpp
                                   kmlexportconfig.cpp
   )

KDE4_ADD_PLUGIN(kipiplugin_kmlexport ${kipiplugin_kmlexport_PART_SRCS})

TARGET_LINK_LIBRARIES(kipiplugin_kmlexport
                      ${KDE4_KPARTS_LIBS}
                      ${KIPI_LIBRARIES}
                      ${KEXIV2_LIBRARIES}
                      kipiplugins
                     )

INSTALL(TARGETS kipiplugin_kmlexport DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES kipiplugin_kmlexport.desktop DESTINATION ${SERVICES_INSTALL_DIR})

IF(KDE4_BUILD_TESTS)
    MACRO_OPTIONAL_ADD_SUBDIRECTORY(tests)
ENDIF(KDE4_BUILD_TESTS)
