******************
Sample Compilation
******************

.. highlight:: bash

The first thing that you need to do is to make sure that you have a
clean source code. And second if the sample is still not compiling you
have to provide us with more information.

Let’s start with the first part: “Clean Source Code State”. If you are
familiar with ``git`` you can run ``git status`` to get more
information. Sometimes this won’t be enough therefore we recommend you
the following steps ( They are working on Linux with bash shell. If you
have another OS and shell you should adjust them accordingly).

::

   cd /tmp 
   git clone https://github.com/SmingHub/Sming.git SmingForTest
   cd /tmp/SmingForTest/Sming
   export SMING_HOME=/tmp/SmingForTest/Sming

The commands above will fetch the latest source code in a directory that
should be completely different than the Sming directory that you are
using on a daily basis. Also it points the SMING_HOME variable to the
new temporary directory with the clean source code.

Now let’s go to the second step: “Compile a sample and report issues, if
any”. We will use the Basic_Ssl sample. Before we compile a sample we
need to compile the Sming library. This can be done calling the
following commands:

::

   cd /tmp/SmingForTest/Sming
   export SMING_HOME=/tmp/SmingForTest/Sming
   make clean

The last makes sure to clean any intermediate files or directories. If
we run now ``make``. It will fetch the needed submodules, compile the
code and build a library out of it. In our case we need to compile Sming
with an optional SSL support. In order to compile Sming with SSL we need
to add the :envvar:`ENABLE_SSL` =1 directive. The command that we need to run now
will look like this:

::

   make ENABLE_SSL=1 

**On error** If the compilation stops with an error, please, copy the
output that the command above produces and append it to your bug report.
Now run the same command one more time but with the V=1 directive. This
will produce a more verbose output that can help the Sming developers
figure out the issue.

::

   make ENABLE_SSL=1 V=1 

Make sure to append that output too to your bug report. Tell the Sming
developers also what is your SDK (esp-open-sdk, esp-alt-sdk, …) and
version, operating system & version, git version, make & version, so
that the developers can reproduce your problem.

**On success** It is time to compile the Basic_Ssl sample. Do this by
executing the commands below:

::

   cd /tmp/SmingForTest/samples/Basic_Ssl
   export SMING_HOME=/tmp/SmingForTest/Sming
   make clean
   make

**On error 2** If that compilation fails make sure to append the output
to your bug report. Now compile the sample with the V=1 flags, similar
to the compilation of the Sming library.

::

   cd /tmp/SmingForTest/samples/Basic_Ssl
   export SMING_HOME=/tmp/SmingForTest/Sming
   make V=1
