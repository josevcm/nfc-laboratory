cmake_dependent_option(ENABLE_QUICKTEST "LimeSDR quick test utility" OFF "ENABLE_LIBRARY" OFF)
add_feature_info(LimeQuickTest ENABLE_QUICKTEST "LimeSDR-QuickTest Utility")

if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif()

if(ENABLE_QUICKTEST)
    add_executable(LimeQuickTest
		   LimeSDRTest.cpp
		   LimeSDRTest_Mini.cpp
		   LimeSDRTest_USB.cpp
		   LimeNET_Micro_Test.cpp
        	   TestGUI.cpp
		   resources/resource.rc
#		   ${PROJECT_SOURCE_DIR}/external/kissFFT/kiss_fft.c
			)
    target_link_libraries(LimeQuickTest
		      LimeSuite
		      ${FLTK_LDFLAGS}
		      ${FLTK_LIBRARIES}
		      ${GLUT_LIBRARY}
		      )
    #find FLTK
    set(FLTK_SKIP_FLUID true)
    find_package(FLTK)
    execute_process(COMMAND fltk-config --ldflags OUTPUT_VARIABLE FLTK_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (FLTK_FOUND)
   	include_directories(${FLTK_INCLUDE_DIR})
	target_compile_definitions(LimeQuickTest PRIVATE -DQUICKTEST_GUI)
    	target_link_libraries(LimeQuickTest
		      ${FLTK_LDFLAGS}
		      ${FLTK_LIBRARIES}
		      ${GLUT_LIBRARY}
		      )
    endif()
    install(TARGETS LimeQuickTest DESTINATION bin)
endif()





