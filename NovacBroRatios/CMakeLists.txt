##############################################################
## This is the main project of Novac BrO Ratio Test Project ##
##############################################################

cmake_minimum_required (VERSION 3.6)


## --------- Creating NovacBrORatios ---------

# Add the different components
add_subdirectory(Common)
add_subdirectory(Configuration)
add_subdirectory(Evaluation)

add_executable(NovacBrORatios
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/stdafx.h
    ${CMAKE_CURRENT_LIST_DIR}/Misc.h
    ${CMAKE_CURRENT_LIST_DIR}/Misc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Molecule.h
    ${CMAKE_CURRENT_LIST_DIR}/Molecule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/SetupFileReader.h
    ${CMAKE_CURRENT_LIST_DIR}/SetupFileReader.cpp

    ${NPP_COMMON_HEADERS}
    ${NPP_COMMON_SOURCES}
    ${NPP_EVALUATION_HEADERS}
    ${NPP_EVALUATION_SOURCES}
    ${NPP_CONFIGURATION_HEADERS}
    ${NPP_CONFIGURATION_SOURCES}    
)

target_include_directories(NovacBrORatios PRIVATE 
    ${PppLib_INCLUDE_DIRS}
    ${SPECTRALEVAUATION_INCLUDE_DIRS})

target_link_libraries(NovacBrORatios PRIVATE PPPLib)

IF(WIN32)
    target_compile_options(NovacBrORatios PUBLIC /W4 /WX /sdl)
    target_compile_definitions(NovacBrORatios PUBLIC _CRT_SECURE_NO_WARNINGS)
ELSE()
    target_compile_options(NovacBrORatios PUBLIC -Wall -std=c++14)
ENDIF()


IF(WIN32)
    #Organize the files into folders, for ease of use (Windows and Visual Studio only)
    source_group("Header Files\\Common"                 FILES ${NPP_COMMON_HEADERS})
    source_group("Header Files\\Communication"          FILES ${NPP_COMMUNICATION_HEADERS})
    source_group("Header Files\\Configuration"          FILES ${NPP_CONFIGURATION_HEADERS})
    source_group("Header Files\\Evaluation"             FILES ${NPP_EVALUATION_HEADERS})
    source_group("Header Files\\Filesystem"             FILES ${NPP_FILESYSTEM_HEADERS})
    source_group("Header Files\\Flux"                   FILES ${NPP_FLUX_HEADERS})
    source_group("Header Files\\Geometry"               FILES ${NPP_GEOMETRY_HEADERS})
    source_group("Header Files\\Meteorology"            FILES ${NPP_METEOROLOGY_HEADERS})
    source_group("Header Files\\Stratosphere"           FILES ${NPP_STRATOSPHERE_HEADERS})
    source_group("Header Files\\WindMeasurement"        FILES ${NPP_WINDMEASUREMENT_HEADERS})

    source_group("Source Files\\Common"                 FILES ${NPP_COMMON_SOURCES})
    source_group("Source Files\\Communication"          FILES ${NPP_COMMUNICATION_SOURCES})
    source_group("Source Files\\Configuration"          FILES ${NPP_CONFIGURATION_SOURCES})
    source_group("Source Files\\Evaluation"             FILES ${NPP_EVALUATION_SOURCES})
    source_group("Source Files\\Flux"                   FILES ${NPP_FLUX_SOURCES})
    source_group("Source Files\\Filesystem"             FILES ${NPP_FILESYSTEM_SOURCES})
    source_group("Source Files\\Geometry"               FILES ${NPP_GEOMETRY_SOURCES})
    source_group("Source Files\\Meteorology"            FILES ${NPP_METEOROLOGY_SOURCES})
    source_group("Source Files\\Stratosphere"           FILES ${NPP_STRATOSPHERE_SOURCES})
    source_group("Source Files\\WindMeasurement"        FILES ${NPP_WINDMEASUREMENT_SOURCES})
endif()


