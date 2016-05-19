project(libjfy)

add_executable(libjfy INIReader.cpp ini.c jfycommon.cpp jfydata.cpp jfyserial.cpp jfypacketdata.cpp jfyexception.cpp jfyconnection.cpp jfysocket.cpp clientsocket.cpp main.cpp)

