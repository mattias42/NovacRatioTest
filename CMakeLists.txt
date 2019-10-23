# This is a test project used for developing the Bro/SO2 ratio calculations

cmake_minimum_required (VERSION 3.6)

# Output the executables to the '/bin' directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/../poco/lib/cmake/Poco")

project (NovacBroRatios)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ------------------- Dependencies -------------------
# N/A
    
# ------------------- Components -------------------

# Add the different components
add_subdirectory(SpectralEvaluation)
add_subdirectory(PPPLib)
add_subdirectory(NovacBroRatios)
 