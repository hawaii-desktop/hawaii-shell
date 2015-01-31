include_directories(
    ${CMAKE_SOURCE_DIR}/3rdparty
)

set(SOURCES
    launcheritem.cpp
    launchermodel.cpp
    launcherplugin.cpp
)

add_library(launcherplugin SHARED ${SOURCES})
target_link_libraries(launcherplugin
                      Qt5::Qml
                      Qt5::Quick)
if(QTXDG_FOUND)
    target_link_libraries(launcherplugin ${QTXDG_LIBRARIES})
else()
    target_link_libraries(launcherplugin HawaiiQtXDG)
endif()


install(FILES qmldir
    DESTINATION ${QML_INSTALL_DIR}/org/hawaii/launcher)
install(TARGETS launcherplugin
    DESTINATION ${QML_INSTALL_DIR}/org/hawaii/launcher)
