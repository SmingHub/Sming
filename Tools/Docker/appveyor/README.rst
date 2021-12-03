Appveyor BYOC
=============

.. highlight:: bash

`Bring Your Own Cloud <https://www.appveyor.com/docs/byoc>`__
is an appveyor feature which allows use of local computing resources to perform builds.

Use of pre-built images avoids the overhead of installing tools.

Setup
-----

.. highlight:: bash

1. Install Docker

2. Install appveyor scripts

   Set-ExecutionPolicy Bypass -Scope CurrentUser -Force; Install-Module AppVeyorBYOC -Scope CurrentUser -Force; Import-Module AppVeyorBYOC

3. Configure cloud in appveyor account

   - Appveyor -> BYOC -> Add Cloud

   - Cloud Provider: Docker

   - Operating system: As appropriate

   - Base Image: ``Ubuntu 20.04 Minimal`` or ``Windows Server Core 2019 Minimal``

   - Shell commands::

      git clone https://github.com/SmingHub/Sming --branch develop --depth 1 /tmp/sming
      pwsh /tmp/sming/Tools/Docker/appveyor/setup.ps1

   - Image name, suggest ``linux`` or ``windows`` as appropriate

   Execute commands as indicated in the resulting screen.

4. Customise build cloud

   Custom Docker command arguments
      Customise CPU resources, RAM usage, etc. For example::

         --cpus=8

      See https://docs.docker.com/engine/reference/commandline/run/.

   Images
      Provide mappings between appveyor script image names and docker images.
      Sming requires the following mappings::

         "Ubuntu2004" -> "linux"
         "Visual Studio 2019" -> "windows"

   Failover strategy
      Concurrent jobs are limited so the following settings are suggested::

         Job start timeout: 60
         Provisioning attempts: 100

4. Configure project settings

   - Set ``APPVEYOR_BUILD_WORKER_CLOUD`` environment variable


Rebuilding docker images
------------------------

Appveyor images are customised by pre-installing Sming build tools.
When these are updated images must be re-built.
However, docker will not be able to detect image changes as the setup script (above) doesn't change,
only the scripts in the Sming repository.

Therefore, existing images should be deleted before rebuilding::

   docker image rm appveyor-byoc-ubuntu2004
   docker image rm appveyor-byoc-visual-studio-2019

Alternatively, use the provided dockerfiles::

   docker build --no-cache -t linux -f Dockerfile-Linux .
   docker build --no-cache -t windows -f Dockerfile-Windows .


Custom images
-------------

To use a Sming fork for building the image simply replace the repo URL and branch in the above script.
These may also be passed to docker build as follows::

   docker build -t linux-test -f Dockerfile-Linux --build-arg SMING_REPO=https://github.com/myrepo/Sming --build-arg SMING_BRANCH=feature/appveyor-revisions .
