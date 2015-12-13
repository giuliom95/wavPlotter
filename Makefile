CODE_DIR = src

.PHONY: wavPlotter

wavPlotter:
	$(MAKE) -C $(CODE_DIR)
	mv $(CODE_DIR)/wavPlotter ./
	rm $(CODE_DIR)/*.o
