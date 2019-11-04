****************
Using with CLion
****************

Developing with the Sming framework can also be done in CLion.

1. Copy and paste Makefiles into the project
2. Create ``app`` folder and add ``application.cpp`` there
3. Edit ``CMakeLists.txt``

## Edit CMakeLists.txt

::

   include_directories("/opt/Sming/Sming")
   include_directories("/opt/Sming/Sming/Libraries")
   include_directories("/opt/Sming/Sming/system/include")
   include_directories("/opt/esp-open-sdk/sdk/include")

   set(SOURCE_FILES app/application.cpp)
   add_executable(Pathfinder ${SOURCE_FILES})

Build the project using terminal
--------------------------------

``make && make flash``
