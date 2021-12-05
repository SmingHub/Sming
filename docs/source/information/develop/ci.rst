Continuous Integration Testing
==============================

.. highlight:: bash

It is important to ensure that any change made to framework code does not introduce additional bugs.
In practice, this is impossible to guarantee but we can at least perform a full build of the entire
framework with all samples (both in the framework any any associated libraries).

This is done using the integration testing framework using linux and Windows build environments.

In addition, a number of integration tests are run using Host builds which verify the logic of a large
proportion of the code.
Testing low-level operation requires real hardware and this must be done manually, but in general
libraries and samples can be largely tested using Host builds and carefully constructed tests.

The :library:`SmingTest` library should be used for such test applications to ensure they
are supported on all architectures.
It also provides a mechanism for logging test results.


Appveyor
--------

We use `appveyor <https://ci.appveyor.com>`__ to manage all test builds.
This service is free of charge for open-source projects.

Note: We used to use `Travis <https://travis-ci.org>`__ but this is no longer free of charge.

The build is controlled via the ``appveyor.yml`` file in the sming root directory.
Sming performs the build and test logic is handled using scripts, which are intended to be easily
portable to other CI services if necessary.
Mostly batch scripts (.cmd) are used for Windows, and bash scripts (.sh) for GNU/Linux but
where practical powershell core is used as this runs on either.


.. note::

   Appveyor also supports macOS but at present Sming doesn't perform CI builds on that platform.


Configuration
~~~~~~~~~~~~~

Sming developers may use integration testing for their own projects, libraries or framework changes.

Configure as follows:

-   Visit https://www.appveyor.com/ and create an account. It's usually easiest to sign up using the ``GitHub`` link.
-   Select ``Projects`` from the toolbar and click on ``New Project``. If there are no projects listed make sure
    AppVeyor has been authorised as a GitHub App.
-   You can now click ``New Build`` to build the default branch.
    This may not be what you require so visit the project settings page and configure as necessary.

By default, pull requests are built automatically.

The `Rolling builds <https://www.appveyor.com/docs/build-configuration/#rolling-builds>`__
setting ensures that only the most recent commit to a branch is built, so should usually be enabled.


Library CI support
------------------

Appveyor may be configured to test a Sming library separately. Steps to enable:

Add project to appveyor account
    ``Projects`` -> ``New Project`` and select from list

Set ``Custom Configuration``
    to https://raw.githubusercontent.com/SmingHub/Sming/develop/Tools/ci/library/appveyor.txt.

Set ``Project URL slug``
    If the library under test already exists in the Sming framework then the test directory
    MUST have the same name to ensure it gets picked up.

    For example, testing the ``Sming-jerryscript`` library requires this value to be set to ``jerryscript``
    to match the Sming library name.
    Build logs should then report a warning ``Multiple matches found for Component 'jerryscript'.

Set sming fork/branch
    By default builds use the main Sming ``develop`` branch.
    If testing a library which requires changes to the framework, you'll need to use a fork
    and add ``SMING_REPO`` and ``SMING_BRANCH`` environment variables to the project settings.

    Note that environment variables set here will override any values set in appveyor.txt.

The provided default :source:`makefile <Tools/ci/library/Makefile>`
builds all applications within the library's ``samples`` directory.
If a test application is provided then that should be located in a ``test`` directory.
This is built for all architectures, and also executed for Host.


Build on your own 'cloud'
-------------------------

Resources are limited to one concurrent build job per appveyor account.
Each build gets two virtual CPUs but they're not particular fast.
Network connectivity is, on the other hand, excellent!

One very useful feature that appveyor provides is `Bring Your Own Cloud or Computer <https://www.appveyor.com/docs/byoc/>`__.
This allows the actual builds to be run on other hardware.

Builds can have up to 5 concurrent jobs and as many CPUs as are available.
In addition, build images can be pre-installed with toolchains.
This can reduce total run times from 5+ hours to around 30 minutes.


Configuration
~~~~~~~~~~~~~

Full support requires a Windows server with Hyper-V, WSL2 and `Docker <https://www.docker.com/>`__ installed.
Hyper-V is built into Windows 10/11 professional edition.
WSL2 should be available on all Windows versions.

Linux/MacOS are supported but only for GNU/Linux images.

Note that whilst Docker supports both Windows and Linux images, both cannot be used at the same time:
it is necessary to manually switch between Linux/Windows containers.
However, testing shows much better performance using Hyper-V for Windows builds.

1.  Add Docker build cloud for Linux builds:

    - Appveyor -> BYOC -> Add Cloud

    - Cloud Provider: Docker

    - Operating system: Windows

    - Base Image: ``Ubuntu 20.04 Minimal``

    - Shell commands::

        git clone https://github.com/SmingHub/Sming --branch develop --depth 1 /tmp/sming
        pwsh /tmp/sming/Tools/Docker/appveyor/setup.ps1

    - Image name: ``linux``

    Execute commands as indicated in the resulting screen.

    Wait for the image to be built.

    The final stage updates the cloud information in your appveyor account.
    Customise as follows:

    - Name
        Change this so it contains only letters, numbers and dash (-).
        Default names contain a space, e.g. ``COMPUTER Docker`` so change to ``COMPUTER-Docker``

    - Custom Docker command arguments
        Customise CPU resources, RAM usage, etc. For example::

            --cpus=8

        See https://docs.docker.com/engine/reference/commandline/run/.

    - Failover strategy
        Default values will fail a job if no worker is available to service it.
        The following settings are suggested::

            Job start timeout: 60
            Provisioning attempts: 100


2. Add Hyper-V build cloud for Windows builds:

Same as (1) above except:

- Cloud Provider: Hyper-V
- Base Image: ``Windows Server Core 2019 Minimal``
- Image name: ``windows``

When complete, fix the build cloud name as previously, e.g. ``COMPUTER-HyperV``.
Also check CPU cores, RAM allocation, failover strategy.


3. Fix authorization token

The above steps will also install the Appveyor Host Agent software on your computer.
This is the software which communicates with the Appveyor server and directs the build jobs.

The authorization token used by the agent can be found in the registry:

.. code-block:: text

    Computer\HKEY_LOCAL_MACHINE\SOFTWARE\AppVeyor\HostAgent

Make sure that both clouds have the same token.


4. Configure BYOC images

    Select ``BYOC`` -> ``Images`` and amend mappings as follows:

    (1)

         - Image Name: ``Ubuntu2004``
         - OS Type: ``Linux``
         - Build cloud: "COMPUTER-Docker" (as configured above)

    (2)

         - Image Name: ``Visual Studio 2019``
         - OS Type: ``Windows``
         - Build cloud: "COMPUTER-HyperV" (as configured above)

    Now, when a build is started it should use your own server.
    To revert back to normal operation change the ``Image Name`` fields in the entries.
    It's not necessary to delete them: just add, say, "X" to the name so they're not recognised.


    .. note::

        Clouds may also be configured on a per-project basis by setting the ``APPVEYOR_BUILD_WORKER_CLOUD``
        environment variable to the appropriate cloud name.

        To get both Linux and Windows builds working concurrently using this approach would require a single
        cloud to support dual images.


Rebuilding docker images
~~~~~~~~~~~~~~~~~~~~~~~~

Appveyor images are customised by pre-installing Sming build tools.
When these are updated images must be re-built.

The easiest way to do this is using the provided dockerfiles::

   cd $SMING_HOME/../Tools/Docker/appveyor
   docker build --no-cache -t linux -f Dockerfile-Linux .
   docker build --no-cache -t windows -f Dockerfile-Windows .


Custom images
-------------

To use a Sming fork for building the image simply replace the repo URL and branch in the ``Shell Commands`` given above.

These may also be passed to docker build as follows::

   docker build -t linux-test -f Dockerfile-Linux --build-arg SMING_REPO=https://github.com/myrepo/Sming --build-arg SMING_BRANCH=feature/appveyor-revisions .


Issues
------

If you get error ``image not supported by cloud`` this probably means an image has been mapped to the wrong clould.
Goto Appveyor -> BYOC -> Images and update/delete the offending entries.

If either cloud is shown as ``offline`` then check the authorization token (step 4 above).
It may be necessary to restart the ``Appveyor Host Agent`` service (via Windows service manager).
