

# CMakeList.txt: Projekt CMake pro Client, sem p�idejte logiku zdrojov�ho
# k�du a definic specifickou pro projekt.
#
cmake_minimum_required (VERSION 3.8)

set(ASIO_SOURCE_DIR "C:/Users/danielk/libs/vcpkg/packages/asio_x64-windows/share/asio/")
#set(BOOST_SOURCE_DIR "C:/Program Files/boost/boost_1_60_0")

find_package(asio PATHS "${ASIO_SOURCE_DIR}" CONFIG REQUIRED)

# P�idejte zdrojov� k�d ke spustiteln�mu souboru tohoto projektu.

file(GLOB Server_SOURCES src/*.cpp )
add_executable (Server ${Server_SOURCES})

target_include_directories(Server PRIVATE include)
#target_include_directories(Client PRIVATE "${BOOST_SOURCE_DIR}")

#file(GLOB BOOST_LIBS "C:/Program Files/boost/boost_1_60_0/lib/*.lib")
#target_link_libraries(Client PRIVATE ${BOOST_LIBS})

target_link_libraries(Server PRIVATE asio asio::asio)

# TODO: V p��pad� pot�eby p�idejte testy a c�le instalace.
