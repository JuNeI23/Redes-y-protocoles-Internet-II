# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/duke-nicolas/esp/v5.5.1/esp-idf/components/bootloader/subproject"
  "/home/duke-nicolas/Documents/Ecole/Complutense/Clases/RPI2/Redes-y-protocoles-Internet-II/Practicas_2/wolfssl_client/build/bootloader"
  "/home/duke-nicolas/Documents/Ecole/Complutense/Clases/RPI2/Redes-y-protocoles-Internet-II/Practicas_2/wolfssl_client/build/bootloader-prefix"
  "/home/duke-nicolas/Documents/Ecole/Complutense/Clases/RPI2/Redes-y-protocoles-Internet-II/Practicas_2/wolfssl_client/build/bootloader-prefix/tmp"
  "/home/duke-nicolas/Documents/Ecole/Complutense/Clases/RPI2/Redes-y-protocoles-Internet-II/Practicas_2/wolfssl_client/build/bootloader-prefix/src/bootloader-stamp"
  "/home/duke-nicolas/Documents/Ecole/Complutense/Clases/RPI2/Redes-y-protocoles-Internet-II/Practicas_2/wolfssl_client/build/bootloader-prefix/src"
  "/home/duke-nicolas/Documents/Ecole/Complutense/Clases/RPI2/Redes-y-protocoles-Internet-II/Practicas_2/wolfssl_client/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/duke-nicolas/Documents/Ecole/Complutense/Clases/RPI2/Redes-y-protocoles-Internet-II/Practicas_2/wolfssl_client/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/duke-nicolas/Documents/Ecole/Complutense/Clases/RPI2/Redes-y-protocoles-Internet-II/Practicas_2/wolfssl_client/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
