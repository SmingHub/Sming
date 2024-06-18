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


Github Actions
--------------

We use Github Actions to manage all test builds.
This service is free of charge for open-source projects.

Sming performs the build and test logic is handled using scripts, which are intended to be easily
portable to other CI services if necessary.
Mostly batch scripts (.cmd) are used for Windows, and bash scripts (.sh) for GNU/Linux but
where practical powershell core is used as this runs on either.

.. note::

   Sming doesn't perform CI builds for MacOS.


Library CI support
------------------

Sming libraries may be separately built and tested whether or not they are included as part of
the Sming repository (or a fork).

There is currently only one supported mechanism available: GitHub Actions.

The ``library.yml`` reusable workflow is provided, which takes care of these tasks:

- Checking in the library to test
- Checking in the Sming framework
- Installing build tools
- Builds all applications within the library's ``samples`` directory, for all supported architectures
- If a test application is provided then that should be located in a ``test`` directory.
  This is built for all architectures, and also executed for Host.

Builds are handled using :source:`Tools/ci/library/Makefile`.

See also https://docs.github.com/en/actions/using-workflows/reusing-workflows.

To use this in a project, add a suitable workflow to the ``.github/workflows`` directory.
Templates are provided in the ``.github/workflows/library`` directory.

Here is the basic ``push`` scenario:

.. code-block:: yaml

   name: CI Push
   on: [push]
   jobs:
     build:
       uses: SmingHub/Sming/.github/workflows/library.yml@develop
       # Inputs are all optional, defaults are shown
       with:
         # Repository to fetch Sming from
         sming_repo: 'https://github.com/SmingHub/Sming'
         # Sming branch to run against
         sming_branch: 'develop'
         # Library alias
         alias: ''

The ``sming_repo`` and ``sming_branch`` inputs are provided if your library requires modifications
to Sming which are not (yet) in the main repository.

The ``alias`` input is required where the library repository name does not correspond with
the working title.
For example, the ``jerryscript`` library is in a repository called ``Sming-jerryscript``,
so must be checked out using a different name.
If Sming contains a library (or Component) with the same name then it will be overridden,
with a warning ``Multiple matches found for Component 'jerryscript'`` in the build log.

The ``ci-dispatch.yml`` example demonstrates manual triggering, which allows these inputs to be easily changed.
See https://docs.github.com/en/actions/managing-workflow-runs/manually-running-a-workflow.

Note that the workflow must be available in the library's default branch, or it will
not appear in the github web page.
