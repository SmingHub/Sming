doxygen-bootstrapped
===================

Customize doxygen (v1.8.13) output to use the twitter bootstrap framework (v3.3.6). 

*This fork is aiminig to provide support for doxygen 1.8.12+ which changed the
way that the menus are structured and move to smartmenus. This is work in
progress and should not be used right now.*

[Demo](https://biogearsengine.com/documentation/index.html)

This started as work done by CoActionOS and was extended further here.
[Credit](http://coactionos.com/embedded%20design%20tips/2014/01/07/Tips-Integrating-Doxygen-and-Bootstrap/)

# Customizing Doxygen
Doxygen provides a handful of ways to [customize the output](http://www.stack.nl/~dimitri/doxygen/manual/customize.html). The simplest way is to customize the HTML output.

Doxygen allows you to customize the HTML output by modifying a master HTML header, footer and stylesheet. You can then include additional stylesheets and javascript files. The following command will generate the default Doxygen files.

`doxygen -w html header.html footer.html customdoxygen.css`

Modifying these files alone is not enough to get good Twitter Bootstrap integration. Bootstrap requires that certain classes be applied within the HTML. For example, a Bootstrap "table" needs to have a class called "table" in order to apply the Bootstrap table formatting. We just need to augment the default HTML with these Bootstrap classes. To do this, we use the provided doxy-boot.js javascript file.

Also, you can augment doxygen's default stylesheet with a customdoxygen.css stylesheet. This is where you would place any custom styling such as sticky footers.

# How to Integrate

To integrate this into your own project tell your doxyfile to use these 4 files in the HTML section (see the example site for an example of each file):

* HTML_HEADER=header.html
    * Adds a Bootstrap navbar
    * Wraps the content in a Bootstrap container/row
* HTML_FOOTER=footer.html
    * Closes the extra divs opened in the header.html
* HTML\_EXTRA_STYLESHEET=customdoxygen.css
    * Adds additional styling such as a sticky footer   
* HTML\_EXTRA_FILES=doxy-boot.js
    * Where the magic happens to augment the HTML with bootstrap

NOTE: The header.html file needs to include the Bootstrap css/javascript for this to work. This is where you can specify your own bootstrap compilation. These files will need to be manually added to the html directory, added as additional files in the doxyfile HTML\_EXTRA_FILES section or referenced externally (see example site header.html).

NOTE: If you want to use the customdoxygen.css stylesheet from this repository, then you will need to replace the customdoxygen.css style sheet generated by the doxygen command above. If your customdoxygen.css file is in the directory when you run the doxygen command, it will be moved to customdoxygen.css.bak and you can restore it by overwriting the new version with the backup.

See the example-site directory for a minimal working example.

## Todo List
* Menu is not correctly displayed when Doxygen sidebar is enabled.
