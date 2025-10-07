RELEASE_NUMBER=2025_10

release: release_win release_linux

release_linux: check_KCC/check_KCC  AUTO_Start_8k/AUTO_Start_8k  AUTO_Start_16k/AUTO_Start_16k   JUMP_Start_8k/JUMP_Start_8k  JUMP_Start_16k/JUMP_Start_16k  MENU_Start_8k/MENU_Start_8k  MENU_Start_16k/MENU_Start_16k
	mkdir -p release_linux
	cp check_KCC/check_KCC           release_linux
	cp AUTO_Start_8k/AUTO_Start_8k   release_linux
	cp AUTO_Start_16k/AUTO_Start_16k release_linux
	cp JUMP_Start_8k/JUMP_Start_8k   release_linux
	cp JUMP_Start_16k/JUMP_Start_16k release_linux
	cp MENU_Start_8k/MENU_Start_8k   release_linux
	cp MENU_Start_16k/MENU_Start_16k release_linux
	cd release_linux; zip -9   ../release_linux__$(RELEASE_NUMBER) *

release_win: check_KCC/check_KCC.exe  AUTO_Start_8k/AUTO_Start_8k.exe  AUTO_Start_16k/AUTO_Start_16k.exe   JUMP_Start_8k/JUMP_Start_8k.exe  JUMP_Start_16k/JUMP_Start_16k.exe  MENU_Start_8k/MENU_Start_8k.exe  MENU_Start_16k/MENU_Start_16k.exe
	mkdir -p release_win
	cp check_KCC/check_KCC.exe           release_win
	cp AUTO_Start_8k/AUTO_Start_8k.exe   release_win
	cp AUTO_Start_16k/AUTO_Start_16k.exe release_win
	cp JUMP_Start_8k/JUMP_Start_8k.exe   release_win
	cp JUMP_Start_16k/JUMP_Start_16k.exe release_win
	cp MENU_Start_8k/MENU_Start_8k.exe   release_win
	cp MENU_Start_16k/MENU_Start_16k.exe release_win
	cd release_win; zip -9   ../release_win__$(RELEASE_NUMBER) *


check_KCC/check_KCC:
	$(MAKE) -C check_KCC check_KCC

AUTO_Start_8k/AUTO_Start_8k:
	$(MAKE) -C AUTO_Start_8k all

AUTO_Start_16k/AUTO_Start_16k:
	$(MAKE) -C AUTO_Start_16k all

JUMP_Start_8k/JUMP_Start_8k:
	$(MAKE) -C JUMP_Start_8k all

JUMP_Start_16k/JUMP_Start_16k:
	$(MAKE) -C JUMP_Start_16k all

MENU_Start_8k/MENU_Start_8k:
	$(MAKE) -C MENU_Start_8k all

MENU_Start_16k/MENU_Start_16k:
	$(MAKE) -C MENU_Start_16k all


check_KCC/check_KCC.exe:
	$(MAKE) -C check_KCC check_KCC.exe

AUTO_Start_8k/AUTO_Start_8k.exe:
	$(MAKE) -C AUTO_Start_8k exe

AUTO_Start_16k/AUTO_Start_16k.exe:
	$(MAKE) -C AUTO_Start_16k exe

JUMP_Start_8k/JUMP_Start_8k.exe:
	$(MAKE) -C JUMP_Start_8k exe

JUMP_Start_16k/JUMP_Start_16k.exe:
	$(MAKE) -C JUMP_Start_16k exe

MENU_Start_8k/MENU_Start_8k.exe:
	$(MAKE) -C MENU_Start_8k exe

MENU_Start_16k/MENU_Start_16k.exe:
	$(MAKE) -C MENU_Start_16k exe


.phony: release_win release_linux

clean:
	rm -rf release_linux
	rm -rf release_win
	$(MAKE) clean -C check_KCC
	$(MAKE) clean -C AUTO_Start_8k
	$(MAKE) clean -C AUTO_Start_16k
	$(MAKE) clean -C JUMP_Start_8k
	$(MAKE) clean -C JUMP_Start_16k
	$(MAKE) clean -C MENU_Start_8k
	$(MAKE) clean -C MENU_Start_16k
