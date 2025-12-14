cmake_minimum_required(VERSION 3.15)
project(pwm)

# 设置为SDCC编译器
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_COMPILER sdcc)
set(CMAKE_C_COMPILER_ID SDCC)
set(CMAKE_EXECUTABLE_SUFFIX .ihx)

# 禁用CMake的默认编译规则（因为SDCC不使用.o文件）
set(CMAKE_C_COMPILE_OBJECT "")
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <OBJECTS> -o <TARGET>")

# 设置SDCC特有的编译选项
set(CMAKE_C_FLAGS "")

# 源文件列表
set(SOURCES ${CMAKE_SOURCE_DIR}/src/main.c)

# 自定义编译命令
add_custom_command(
    OUTPUT ${PROJECT_NAME}.ihx
    COMMAND sdcc ${CMAKE_C_FLAGS} ${SOURCES} -o ${PROJECT_NAME}.ihx
    DEPENDS ${SOURCES}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "编译${PROJECT_NAME}项目"
)

# 生成HEX文件
add_custom_command(
    OUTPUT ${PROJECT_NAME}.hex
    COMMAND packihx ${PROJECT_NAME}.ihx > ${PROJECT_NAME}.hex
    DEPENDS ${PROJECT_NAME}.ihx
    COMMENT "生成HEX文件"
)

# 主目标
add_custom_target(${PROJECT_NAME} ALL DEPENDS ${PROJECT_NAME}.hex)

# 下载目标
add_custom_target(flash
    COMMAND stcgal -p /dev/ttyUSB0 -a ${PROJECT_NAME}.hex
    DEPENDS ${PROJECT_NAME}.hex
    COMMENT "下载到STC8G1K08A"
)

# 清理命令
add_custom_target(clean_all
    COMMAND rm -f *.ihx *.hex *.lk *.map *.mem *.rst
    COMMENT "清理所有生成文件"
)
