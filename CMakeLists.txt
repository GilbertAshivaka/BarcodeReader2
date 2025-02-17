cmake_minimum_required(VERSION 3.16)
project(BarcodeReader2 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(TARGET_NAME appBarcodeReader2)

find_package(Qt6 6.4 REQUIRED COMPONENTS Quick Multimedia)

qt_standard_project_setup()

qt_add_executable(${TARGET_NAME}
    main.cpp
    include/DynamsoftBarcodeReader.h include/DynamsoftCaptureVisionRouter.h include/DynamsoftCodeParser.h include/DynamsoftCore.h include/DynamsoftDocumentNormalizer.h include/DynamsoftImageProcessing.h include/DynamsoftLabelRecognizer.h include/DynamsoftLicense.h include/DynamsoftUtility.h
)

qt_add_qml_module(${TARGET_NAME}
    URI BarcodeReader2
    VERSION 1.0
    QML_FILES Main.qml
    SOURCES
        include/DynamsoftBarcodeReader.h
        include/DynamsoftCaptureVisionRouter.h
        include/DynamsoftCodeParser.h
        include/DynamsoftCore.h
        include/DynamsoftDocumentNormalizer.h
        include/DynamsoftImageProcessing.h
        include/DynamsoftLabelRecognizer.h
        include/DynamsoftLicense.h
        include/DynamsoftUtility.h
)

set_target_properties(${TARGET_NAME} PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    WIN32_EXECUTABLE TRUE
)

# Include directories for Dynamsoft headers
include_directories("${PROJECT_SOURCE_DIR}/include/")

# Link Qt libraries
target_link_libraries(${TARGET_NAME}
    PRIVATE
        Qt6::Quick
        Qt6::Multimedia
)

# Windows-specific Dynamsoft library configurations
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(${TARGET_NAME}
        PRIVATE "${PROJECT_SOURCE_DIR}/Lib/Windows/x64/DynamsoftBarcodeReaderx64.dll"
    )
else() # Assuming MSVC or other non-GNU compilers
    target_link_libraries(${TARGET_NAME}
        PRIVATE "DBRx64"
    )
endif()

# Copy DLLs to build directory after build
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${PROJECT_SOURCE_DIR}/Lib/Windows/"
    $<TARGET_FILE_DIR:${TARGET_NAME}>
)

include(GNUInstallDirs)
install(TARGETS ${TARGET_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
