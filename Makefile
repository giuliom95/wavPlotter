WAV_PLOTTER_DIR = src
FFTW_DIR = fftw_src
FUNC_GEN_DIR = funcGen_src

.PHONY: wavPlotter
wavPlotter:
	$(MAKE) -C $(WAV_PLOTTER_DIR)
	mv $(WAV_PLOTTER_DIR)/wavPlotter ./
	rm $(WAV_PLOTTER_DIR)/*.o

.PHONY: fftw
fftw:
	$(MAKE) -C $(FFTW_DIR)
	mv $(FFTW_DIR)/fftw ./
	rm $(FFTW_DIR)/*.o
	
.PHONY: funcGen
funcGen:
	$(MAKE) -C $(FUNC_GEN_DIR)
	mv $(FUNC_GEN_DIR)/funcGen ./
