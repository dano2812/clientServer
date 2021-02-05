# CMakeList.txt: Projekt CMake pro Client, sem přidejte logiku zdrojového
# kódu a definic specifickou pro projekt.
#
cmake_minimum_required (VERSION 3.8)

set(ASIO_SOURCE_DIR "C:/Users/danielk/libs/vcpkg/packages/asio_x64-windows/share/asio/")
#set(BOOST_SOURCE_DIR "C:/Program Files/boost/boost_1_60_0")

find_package(asio PATHS "${ASIO_SOURCE_DIR}" CONFIG REQUIRED)

# Přidejte zdrojový kód ke spustitelnému souboru tohoto projektu.

file(GLOB Client_SOURCES src/*.cpp )
add_executable (Client ${Client_SOURCES})

target_include_directories(Client PRIVATE include)
#target_include_directories(Client PRIVATE "${BOOST_SOURCE_DIR}")

#file(GLOB BOOST_LIBS "C:/Program Files/boost/boost_1_60_0/lib/*.lib")
#target_link_libraries(Client PRIVATE ${BOOST_LIBS})

target_link_libraries(Client PRIVATE asio asio::asio)

# TODO: V případě potřeby přidejte testy a cíle instalace.
