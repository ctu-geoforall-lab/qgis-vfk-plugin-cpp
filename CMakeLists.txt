
########################################################
# Files

SET (vfkplugin_SRCS
     vlastnicisearchform.cpp
     vfktextbrowser.cpp
     vfktablemodel.cpp
     vfkmainwindow.cpp
     searchformcontroller.cpp
     parcelysearchform.cpp
     latexdocument.cpp
     jednotkysearchform.cpp
     htmldocument.cpp
     richtextdocument.cpp
     domains.cpp
     documentbuilder.cpp
     budovysearchform.cpp
     vfkplugin.cpp
     vfkplugingui.cpp
)

SET (vfkplugin_UIS budovysearchform.ui jednotkysearchform.ui parcelysearchform.ui vfkmainwindow.ui vlastnicisearchform.ui)

SET (vfkplugin_MOC_HDRS
     vlastnicisearchform.h
     vfktextbrowser.h
     vfktablemodel.h
     vfkmainwindow.h
     searchformcontroller.h
     parcelysearchform.h
     jednotkysearchform.h
     budovysearchform.h
     vfkplugin.h
     vfkplugingui.h
)

#     vfkdocument.h
#     htmldocument.h
#     richtextdocument.h
#     domains.h
#     documentbuilder.h
#     latexdocument.h


SET (vfkplugin_RCCS  vfkplugin.qrc)

########################################################
# Build

QT4_WRAP_UI (vfkplugin_UIS_H  ${vfkplugin_UIS})

QT4_WRAP_CPP (vfkplugin_MOC_SRCS  ${vfkplugin_MOC_HDRS})

QT4_ADD_RESOURCES(vfkplugin_RCC_SRCS ${vfkplugin_RCCS})

ADD_LIBRARY (vfkpluginplugin MODULE ${vfkplugin_SRCS} ${vfkplugin_MOC_SRCS} ${vfkplugin_RCC_SRCS} ${vfkplugin_UIS_H})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_SOURCE_DIR}
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core ../../core/raster ../../core/renderer ../../core/symbology
     ../../gui
     ..
     ${GDAL_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(vfkpluginplugin
  qgis_core
  qgis_gui
${GDAL_LIBRARY}
)


########################################################
# Install

INSTALL(TARGETS vfkpluginplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

