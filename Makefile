RELEASE_NUMBER=2025_09

release: release_win release_linux

release_linux: check_KCC/check_KCC
	mkdir -p release_linux
	cp check_KCC/check_KCC release_linux
	cd release_linux; zip -9   ../release_linux__$(RELEASE_NUMBER) *

release_win: check_KCC/check_KCC.exe
	mkdir -p release_win
	cp check_KCC/check_KCC.exe release_win
	cd release_win; zip -9   ../release_win__$(RELEASE_NUMBER) *


check_KCC/check_KCC:
	$(MAKE) -C check_KCC check_KCC

check_KCC/check_KCC.exe:
	$(MAKE) -C check_KCC check_KCC.exe

.phony: release_win release_linux

clean:
	rm -rf release_linux
	rm -rf release_win
	$(MAKE) clean -C check_KCC
