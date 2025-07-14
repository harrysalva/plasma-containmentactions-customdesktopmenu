cmake_minimum_required(VERSION 3.16)

project(plasma-containmentactions-customdesktopmenu)

set(QT_MIN_VERSION "6.6.0")
set(KF6_MIN_VERSION "6.0.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED NO_MODULE)
find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS I18n KIO)
find_package(Plasma ${PROJECT_DEP_VERSION} REQUIRED)

# Try to find PlasmaActivities and PlasmaActivitiesStats (Plasma 6 naming)
find_package(PlasmaActivities QUIET)
find_package(PlasmaActivitiesStats QUIET)

if(PlasmaActivities_FOUND AND PlasmaActivitiesStats_FOUND)
    message(STATUS "PlasmaActivities and PlasmaActivitiesStats found - enabling full favorites support")
    add_definitions(-DHAVE_KACTIVITIES)
    set(KACTIVITIES_LIBS Plasma::Activities Plasma::ActivitiesStats)
else()
    # Fallback: try KF6 naming (for older systems)
    find_package(KF6 QUIET OPTIONAL_COMPONENTS Activities ActivitiesStats)
    if(KF6Activities_FOUND AND KF6ActivitiesStats_FOUND)
        message(STATUS "KF6Activities and KF6ActivitiesStats found - enabling full favorites support")
        add_definitions(-DHAVE_KACTIVITIES)
        set(KACTIVITIES_LIBS KF6::Activities KF6::ActivitiesStats)
    else()
        message(STATUS "KActivities or KActivitiesStats not found - using fallback favorites")
        set(KACTIVITIES_LIBS "")
    endif()
endif()

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_definitions(-DTRANSLATION_DOMAIN=\"plasma_containmentactions_customdesktopmenu\")

set(customdesktopmenu_SRCS
    customdesktopmenu.cpp
)
ki18n_wrap_ui(customdesktopmenu_SRCS config.ui)

kcoreaddons_add_plugin(plasma_containmentactions_customdesktopmenu SOURCES ${customdesktopmenu_SRCS} INSTALL_NAMESPACE "plasma/containmentactions")

set_target_properties(plasma_containmentactions_customdesktopmenu PROPERTIES
    OUTPUT_NAME matmoul-customdesktopmenu
)

target_link_libraries(plasma_containmentactions_customdesktopmenu Plasma::Plasma KF6::KIOCore KF6::KIOWidgets KF6::I18n ${KACTIVITIES_LIBS})
