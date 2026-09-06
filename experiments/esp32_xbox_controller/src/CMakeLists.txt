if(WITTLE_EXP003)
    set(EXPERIMENT_SOURCE "exp003.cpp")
else()
    set(EXPERIMENT_SOURCE "main.cpp")
endif()
idf_component_register(SRCS "${EXPERIMENT_SOURCE}" INCLUDE_DIRS "."
                       REQUIRES bluepad32 btstack driver esp_timer)
