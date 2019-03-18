#Extracted from: https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
APPS_DIR=$(wildcard */)

.PHONY: all clean $(APPS_DIR)

apps: $(APPS_DIR)

clean: cmd_clear $(APPS_DIR)

cmd_clear:
	$(eval CMD=clean)

$(APPS_DIR):
	@make $(CMD) -C $@