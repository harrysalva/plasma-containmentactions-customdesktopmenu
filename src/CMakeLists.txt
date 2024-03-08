cmake_minimum_required(VERSION 3.16)

project(plasma-containmentactions-customdesktopmenu)
set(PROJECT_VERSION "6.0.80")
string(REPLACE "." ";" VERSION_LIST ${PROJECT_VERSION})
list(GET VERSION_LIST 0 PROJECT_VERSION_MAJOR)
list(GET VERSION_LIST 1 PROJECT_VERSION_MINOR)
list(GET VERSION_LIST 2 PROJECT_VERSION_PATCH)

set(PROJECT_DEP_VERSION "6.0.0")
set(QT_MIN_VERSION "6.6.0")
set(KF6_MIN_VERSION "6.0.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")
set(INSTALL_SDDM_THEME TRUE)
option(PLASMA_X11_DEFAULT_SESSION "Use X11 session by default for Plasma" OFF)
option(INSTALL_SDDM_WAYLAND_SESSION OFF)
option(WITH_X11 "Build with X11 support. Building without is experimental" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Svg Widgets Quick QuickWidgets Concurrent Network Core5Compat DBus ShaderTools)
find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
                    Auth Parts Runner Notifications NotifyConfig NewStuff Wallet IdleTime
                    Svg Declarative I18n KCMUtils TextWidgets Crash GlobalAccel DBusAddons
                    CoreAddons KIO Prison Package
                    GuiAddons Archive ItemModels IconThemes UnitConversion TextEditor StatusNotifierItem
                    OPTIONAL_COMPONENTS DocTools)
find_package(Plasma ${PROJECT_DEP_VERSION} REQUIRED)

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

target_link_libraries(plasma_containmentactions_customdesktopmenu Plasma::Plasma KF6::KIOCore KF6::KIOWidgets KF6::I18n)
