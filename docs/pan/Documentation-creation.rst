Documenting code
================

The fullest comment block for our wiki looks like this one

::

   /*!
   * \brief Brief description
   * \param[out] dest arg description
   * \param[in] src arg description
   * \return what sub return
   * \author[s] YourNickname in github [email]
   * \date
   * \version
   * \bug
   * \warning
   * \copyright
   * \example
   * \todo
   * 
   * Full description
   */

   void method_or_function_name () {
   }

To create block this automaticaly(for Eclipse): \*
*Preferences->C/C++->Editor->Documentation tool comments->Workspace
default: select Doxygen* \* Type ``/**`` above target function \* Press
Enter

Groups
======

This block should be before function/method code. To group files or
classes use tags

::

   \defgroup [groupnick] [froup description] 

to define group and

::

   \addtogroup [groupnick] 

to add to group

Note that group should be defined only once per project. If some module
is submodule for another one use

::

   \addtogroup [groupnick] 
       \ingoup [parent_gtoupnick]

There is lots of other tags and methods, but I hope above tags should be
enough

--------------

Doxygen generation and Github wiki page
=======================================

Now we need to generate html pages and put to wiki. First we create wiki
on github

1. Go to https://github.com/[yournick]/Sming/settings -> Automatic page
   generator -> Launch automatic page generator.
2. Select template whatever you want.
3. Install doxygen from
   `here <http://www.stack.nl/~dimitri/doxygen/download.html>`__
4. Go to Sming repo dir. Put `this
   file <https://github.com/Bravo13/Sming/blob/master/Sming/Doxyfile>`__
   to dir

Now create repo with wiki site. Thic command help you

::

   git clone -b gh-pages https://github.com/[yournick]/Sming.git ./../Sming-wiki

And finally do

::

   doxygen

Generated html where in Sming-wiki dir. Now you can push changes from
dir Sming-wiki and Whola - you have site
http://[yournick].github.io/Sming/

More details:
https://help.github.com/articles/creating-project-pages-manually/
