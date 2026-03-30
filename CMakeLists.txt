# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# "Trim" the build. Include the minimal set of components, main, and anything it depends on.
# idf_build_set_property(MINIMAL_BUILD ON)

project(xls2ModbusReg)

# set(EXTRA_COMPONENT_DIRS components)

# 自定义目标，执行Python脚本
add_custom_target(
    run_script_xls2mbreg ALL
    COMMAND python3 ${PROJECT_DIR}/scripts/xls2mbreg.py ${PROJECT_DIR}/mbreg.xlsm ${PROJECT_DIR}/main/pou/mbreg
    WORKING_DIRECTORY ${PROJECT_DIR}
    COMMENT "Running Python script xls2mbreg.py..."
)

# 让主目标依赖于自定义目标
add_dependencies(${PROJECT_NAME}.elf run_script_xls2mbreg)
