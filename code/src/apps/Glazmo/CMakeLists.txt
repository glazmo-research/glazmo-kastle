####### EDIT STUFF BELOW THIS LINE ######

set(APP_NAME "glazmo")
set(APP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Glazmo.cpp
)

###### KEEP EVERYTHING BELOW THIS LINE #######
###### (unless you know what you're doing) ###

# Define the executable
add_executable(${APP_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

# Add the sources
target_sources(${APP_NAME} PRIVATE
    ${KASTLE2_COMMON_SOURCES}
    ${APP_SOURCES}
)

# Link the libraries
target_link_libraries(${APP_NAME} PRIVATE
    ${KASTLE2_COMMON_LIBRARIES}
    ${KASTLE2_CORE_LIBRARY}
)

# Disable stdio through UART and USB - we need those for other things
pico_enable_stdio_uart(${APP_NAME} 0)
pico_enable_stdio_usb(${APP_NAME} 0)

# Set custom properties for the executable
pico_set_program_name(${APP_NAME} ${APP_NAME})

# Generate UF2, HEX, BIN, J-Link script files
generate_output_files(${APP_NAME})
