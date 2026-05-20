# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)
# set(EXTRA_COMPONENT_DIRS 
#         components/Service/yolo26
#         )

set(EXTRA_COMPONENT_DIRS 
        components/BSP/LED
        )

list(APPEND EXTRA_COMPONENT_DIRS
    components/BSP/ADC
    components/BSP/ESPTIMER
    components/BSP/EXTI
    components/BSP/LEDC
    components/BSP/SDMMC_MY
    components/BSP/Serial
    components/BSP/UART
)


list(APPEND EXTRA_COMPONENT_DIRS
    components/Middlewares/motor
    components/Middlewares/MPU6050_XiaoHei
    components/Middlewares/Oled
    components/Middlewares/esp8266
)

# list(APPEND EXTRA_COMPONENT_DIRS
#     components/Service/My_printf
#     components/Service/Filter
#     components/Service/PID
#     components/Service/yolo26
#     components/Service/cat_detect
#     )
list(APPEND EXTRA_COMPONENT_DIRS
    components/Service/My_printf
    components/Service/Filter
    components/Service/PID
    components/Service/yolo26
    components/Service/human_fall_detect
    )


list(APPEND EXTRA_COMPONENT_DIRS
    components/User/Car)

add_compile_options(-fdiagnostics-color=always)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(atk)
