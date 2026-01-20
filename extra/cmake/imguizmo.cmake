set(BUILD_WITH_IMGUIZMO_DEFAULT TRUE)

option(BUILD_WITH_IMGUIZMO "imguizmo Enabled" ${BUILD_WITH_IMGUIZMO_DEFAULT})
message("BUILD_WITH_IMGUIZMO: ${BUILD_WITH_IMGUIZMO}")

if(BUILD_WITH_IMGUIZMO)
  set(IMGUIZMO_DIR ${THIRDPARTY_DIR}/imguizmo)
  set(IMGUIZMO_FILES
    ${IMGUIZMO_DIR}/ImGuizmo.cpp
    ${IMGUIZMO_DIR}/ImGuizmo.h)

  add_library(imguizmo STATIC ${IMGUIZMO_FILES})
  target_include_directories(imguizmo PRIVATE ${IMGUI_DIR})
endif()