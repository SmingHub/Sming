# Generate index.rst file for sample, library or Component
# This provides a template to automate as much as possible
# $1 -> name
# $2 -> Path
# $3 -> Source path relative to working root directory (SMINGDIR)
define GenIndex
$(if $(filter %.rst,$(CMP_$2_README)),
.. include:: $(call GetIncludePath,$(CMP_$2_README)),
.. mdinclude:: $(call GetMdIncludePath,$(CMP_$2_README))
)

References
------------
* :source:`Source Code <$3>`
$(if $(findstring $3=,$(SUBMODULE_URLS)),
* This is a submodule: `GIT repository <$(call GetSubmoduleURL,$3)>`__.
)
$(foreach d,$(sort $(COMPONENT_DEPEND_DIRS)),
* :doc:`$d/index` Component
)
$(foreach l,$(sort $(ARDUINO_LIBRARIES)),
* :library:`$l` Library
)

$(if $(COMPONENT_ENVVARS),
Environment Variables
---------------------
$(foreach v,$(COMPONENT_ENVVARS),
* :envvar:`$v`
))

$(foreach m,$(COMPONENT_SUBMODULES),
Submodule: `$m <$(call GetSubmoduleURL,$3/$m)>`__
-----------------------------------------------------------------------------------------------------

.. toctree::

   $m/README

)
endef
