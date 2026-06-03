set(src_dirs    .)

set(include_dirs include)
set(requires
            driver
			UART
			esp_hw_support
			esp_system
			esp_mm)
            
idf_component_register(SRC_DIRS ${src_dirs}
                       INCLUDE_DIRS ${include_dirs}
                       REQUIRES ${requires})

component_compile_options(-ffast-math -O3 -Wno-error=format=-Wno-format)
            