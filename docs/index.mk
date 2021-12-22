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
----------

* :source:`Source Code <$3>`
$(foreach d,$(sort $(CMP_$2_DEPENDS)),
* :doc:`$(call GetDocPath,$d)/index`)

$(if $(CMP_$1_XREF),
Used by
-------
$(foreach c,$(sort $(CMP_$1_XREF)),
* :doc:`$(call GetDocPath,$c)/index` $(call GetComponentType,$c)))

$(if $(CMP_$2_ENVVARS),
Environment Variables
---------------------
$(foreach v,$(CMP_$2_ENVVARS),
* :envvar:`$v`
))

SoC support
-----------
$(foreach s,$(CMP_$2_SOC_DEPENDS),
- $s)


$(foreach m,$(CMP_$2_SUBMODULES),
Submodule: `$m <$(call GetSubmoduleURL,$3/$m)>`__
-----------------------------------------------------------------------------------------------------

.. toctree::

   $m/README

)
endef
