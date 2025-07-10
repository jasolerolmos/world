cmake_minimum_required(VERSION 3.10)

# Nombre del proyecto
project(ProyectoCXX LANGUAGES CXX)

# Establecer estándar de C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Directorios de inclusión adicionales
include_directories(
    /opt/homebrew/Cellar/boost/1.86.0_2/include/
    /opt/homebrew/opt/ncurses/include
    .
)

# Directorios de librerías adicionales
link_directories(
    /usr/local/lib
    /opt/homebrew/Cellar/boost/1.87.0/lib/
    /opt/homebrew/opt/ncurses/lib
)

# Buscar OpenCV usando pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(OpenCV REQUIRED opencv4)
find_package(OpenCV REQUIRED)

# Agregar los flags de OpenCV (automáticamente manejados por pkg-config)
include_directories(${OpenCV_INCLUDE_DIRS})
link_libraries(${OpenCV_LIBRARIES})

# Librerías adicionales
set(BOOST_LIBS boost_system)
set(NCURSES_LIBS ncurses)
set(THREAD_LIBS pthread)

# Crear ejecutable gui
add_executable(gui gui2.cpp Client.cpp)
target_link_libraries(gui ${BOOST_LIBS} ${NCURSES_LIBS} ${THREAD_LIBS})

# Crear ejecutable server3
add_executable(servidor server3.cpp Personaje.cpp World.cpp PerlinNoise.cpp)
target_link_libraries(servidor ${BOOST_LIBS} ${THREAD_LIBS})

# Crear ejecutable cliente
add_executable(cliente Client.cpp cliente3.cpp)
target_link_libraries(cliente ${BOOST_LIBS} ${THREAD_LIBS})