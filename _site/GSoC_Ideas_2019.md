Google Summer of Code Ideas
----------------------------
# Write a small, in size, Progressive Web App(PWA) to control IoT devices.

The application will be used as a part of a new sample that will help the Sming users to kickstart their development.
The application should be served from a ESP8266 microcontroller and must fit inside it. 
This means that the whole JavaScript, HTML, images and other assets must take less than 500K and require maximum 3 files to be loaded simultaneously. 

The task of the student is to find an existing JavaScript framework that can be used as a basis for the PWA and create the actual PWA. The server-side code is NOT part of the student responsibilities. This is something that the mentors will provide to the student. 

Requirements: HTML, JavaScript and very good understanding how web applications work and more specifically Progressive Web Applications.

# Support for multiple platforms

Sming is very beloved framework due to its elegance and efficiency. But at the current moment Sming Framework is supporting only one microcontroller. Namely ESP8266. We would like to change that in the future and make Sming support other platforms. 

The task of the student will be to analyze the current code base and suggest strategies and code implementation to migrate the code to a new microcontroller ESP32.

Requirements: Very good C/C++ knowledge and experience in making other C/C++ applications platform-independent.

# Pluggable Virtual File System for Sming

At the moment Sming supports reading static data from Flash memory, SD cards and other medias. We would like to unify this access and create a simple, yet fast and CPU and memory efficient, Virtual File System (VFS) that will allow the Sming users to access files and directories without knowing the underlying details as drivers and storage media.

The task of the student will be to draft the VFS, create the initial implementation and add plugins for Spiffs and FATFS file systems.

Requirements: Strong C/C++ skills and an intermediate knowledge related to file systems
