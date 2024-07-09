Using with CLion
****************

.. note::

   This information is no longer current.

   However, if you are able to set up an external ``make`` project with CLion that should work.
   
   To get intelliense (or whatever the equivalent for CLion is) will require setting up a list of the correct #include paths.
   For vscode we have a tool to handle that:

   - run ``make ide-vscode`` from a sample project directory
   - examine generated path list in ``.vscode/c_cpp_properties.json``

   Something similar could be done for CLion perhaps?


Developing with the Sming framework can also be done in CLion.

1. Copy and paste Makefiles into the project
2. Create ``app`` folder and add ``application.cpp`` there
3. Edit ``CMakeLists.txt``

## Edit CMakeLists.txt

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
