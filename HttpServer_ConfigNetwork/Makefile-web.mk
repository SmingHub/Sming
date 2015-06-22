web_dst := files
css_files := $(shell find web/dev/ -name \*.css)
js_files := $(shell find web/dev/ -name \*.js)
html_files := $(shell find web/dev/ -name \*.html)

.PHONY: ${css_files}
${css_files}:
	@yuicompressor -o $(web_dst)/$(notdir $(basename $@)).css $@
	@gzip -9 -f $(web_dst)/$(notdir $(basename $@)).css

.PHONY: ${js_files}
${js_files}:
	@yuicompressor -o $(web_dst)/$(notdir $(basename $@)).js $@
	@gzip -9 -f $(web_dst)/$(notdir $(basename $@)).js

.PHONY: ${html_files}
${html_files}:
	@cp $@ $(web_dst)/$(notdir $@)

.PHONY: ${web_dst}
${web_dst}: 
	@mkdir -p $@

.PHONY: web
web: ${web_dst} ${css_files} ${js_files} ${html_files}
	@echo "HTML, CSS and JS packing done"

.PHONY: clean-web
clean-web:
	@rm -rf ${web_dst}
