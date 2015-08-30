cmake_minimum_required(VERSION 3.0)

########## Variables ##########
set(PVRFRAMEWORK_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/)
set(PVRFRAMEWORK_BUILD_NAME "PVRFramework")
project(${PVRFRAMEWORK_BUILD_NAME})

########## Source ##########
set(PVRFRAMEWORK_SRC
    ${PVRFRAMEWORK_ROOT}/PVRTBoneBatch.cpp
    ${PVRFRAMEWORK_ROOT}/PVRTError.cpp
    ${PVRFRAMEWORK_ROOT}/PVRTFixedPoint.cpp
    ${PVRFRAMEWORK_ROOT}/PVRTMatrixF.cpp
    ${PVRFRAMEWORK_ROOT}/PVRTMatrixX.cpp
    ${PVRFRAMEWORK_ROOT}/PVRTMisc.cpp
    ${PVRFRAMEWORK_ROOT}/PVRTModelPOD.cpp
    ${PVRFRAMEWORK_ROOT}/PVRTQuaternionF.cpp
    ${PVRFRAMEWORK_ROOT}/PVRTQuaternionX.cpp
    ${PVRFRAMEWORK_ROOT}/PVRTResourceFile.cpp
    ${PVRFRAMEWORK_ROOT}/PVRTString.cpp
    ${PVRFRAMEWORK_ROOT}/PVRTTrans.cpp
    ${PVRFRAMEWORK_ROOT}/PVRTVector.cpp
    ${PVRFRAMEWORK_ROOT}/PVRTVertex.cpp
    )
source_group("framework" FILES ${PVRFRAMEWORK_ROOT})

########## Includes ##########
include_directories(${PVRFRAMEWORK_ROOT})

add_library(${PVRFRAMEWORK_BUILD_NAME} STATIC ${PVRFRAMEWORK_SRC})
