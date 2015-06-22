web_dst := files
css_files := $(shell find web/dev/ -name \*.css)
js_files := $(shell find web/dev/ -name \*.js)
html_files := $(shell find web/dev/ -name \*.html)

.PHONY: ${css_files}
${css_files}:
	@yuicompressor -o $(web_dst)/$(notdir $(basename $@))-min.css $@
	@gzip -9 -f $(web_dst)/$(notdir $(basename $@))-min.css

.PHONY: ${js_files}
${js_files}:
	@yuicompressor -o $(web_dst)/$(notdir $(basename $@))-min.js $@
	@gzip -9 -f $(web_dst)/$(notdir $(basename $@))-min.js

.PHONY: ${html_files}
${html_files}:
	@cp $@ $(web_dst)/$(notdir $@)
	sed -r 's/script src=\"([a-zA-Z0-9]+).js\"/script src=\"\1-min.js.gz\"/' $(web_dst)/$(notdir $@) > $(web_dst)/$(notdir $@).new
	mv $(web_dst)/$(notdir $@).new $(web_dst)/$(notdir $@)
	sed -r 's/link href=\"([a-zA-Z0-9]+).css\"/link href=\"\1-min.css.gz\"/' $(web_dst)/$(notdir $@) > $(web_dst)/$(notdir $@).new
	mv $(web_dst)/$(notdir $@).new $(web_dst)/$(notdir $@)
 
.PHONY: web
web: ${css_files} ${js_files} ${html_files}
	@echo "HTML, CSS and JS packing done"

