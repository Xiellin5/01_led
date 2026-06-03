set(src_dirs    .
                lib
                task)

set(include_dirs include)

set(requires
            driver
            human_fall_detect
			Camera
			)
# 传感器
list(APPEND requires
            MPU6050_XiaoHei)

# 运动驱动            
list(APPEND requires
            motor
            PID
            Filter)
            
# 显示器(通信)
list(APPEND requires
            esp8266
            Oled)
            
idf_component_register(SRC_DIRS ${src_dirs}
                       INCLUDE_DIRS ${include_dirs}
                       REQUIRES ${requires})

component_compile_options(-ffast-math -O3 -Wno-error=format=-Wno-format)
            