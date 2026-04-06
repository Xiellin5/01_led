set(src_dirs    .
                eMPL)

set(include_dirs .
                 .
                 eMPL)
set(requires
            driver
            espressif__mpu6050
            unity)
            
idf_component_register(SRC_DIRS ${src_dirs}
                       INCLUDE_DIRS ${include_dirs}
                       REQUIRES ${requires})

component_compile_options(-ffast-math -O3 -Wno-error)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-maybe-uninitialized)
            