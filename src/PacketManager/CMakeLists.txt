file(GLOB packetManager_src "*.cpp")

add_library(serial_packet_manager ${packetManager_src})
# target_include_directories(SerialPacketManager PUBLIC ./)
# target_compile_features(SerialPacketManager PUBLIC cxx_std_11)

# target_link_libraries(serial_packet_manager serial_port)
target_link_libraries(serial_packet_manager async_serial ${CMAKE_THREAD_LIBS_INIT})
