RELEASE_NUMBER=2025_10

release: release_win release_linux

release_linux: check_KCC/check_KCC  AUTO_Start/AUTO_Start  JUMP_Start/JUMP_Start  MENU_Start/MENU_Start  MULTI_Start/MULTI_Start
	mkdir -p release_linux
	cp check_KCC/check_KCC           release_linux
	cp AUTO_Start/AUTO_Start         release_linux
	cp JUMP_Start/JUMP_Start         release_linux
	cp MENU_Start/MENU_Start         release_linux
	cp MULTI_Start/MULTI_Start       release_linux
	cd release_linux; zip -9   ../release_linux__$(RELEASE_NUMBER) *

release_win: check_KCC/check_KCC.exe  AUTO_Start/AUTO_Start.exe  JUMP_Start/JUMP_Start.exe  MENU_Start/MENU_Start.exe  MULTI_Start/MULTI_Start.exe
	mkdir -p release_win
	cp check_KCC/check_KCC.exe           release_win
	cp AUTO_Start/AUTO_Start.exe         release_win
	cp JUMP_Start/JUMP_Start.exe         release_win
	cp MENU_Start/MENU_Start.exe         release_win
	cp MULTI_Start/MULTI_Start.exe       release_win
	cd release_win; zip -9   ../release_win__$(RELEASE_NUMBER) *


check_KCC/check_KCC:
	$(MAKE) -C check_KCC check_KCC

AUTO_Start/AUTO_Start:
	$(MAKE) -C AUTO_Start all

JUMP_Start/JUMP_Start:
	$(MAKE) -C JUMP_Start all

MENU_Start/MENU_Start:
	$(MAKE) -C MENU_Start all

MULTI_Start/MULTI_Start:
	$(MAKE) -C MULTI_Start all


check_KCC/check_KCC.exe:
	$(MAKE) -C check_KCC check_KCC.exe

AUTO_Start/AUTO_Start.exe:
	$(MAKE) -C AUTO_Start exe

JUMP_Start/JUMP_Start.exe:
	$(MAKE) -C JUMP_Start exe

MENU_Start/MENU_Start.exe:
	$(MAKE) -C MENU_Start exe

MULTI_Start/MULTI_Start.exe:
	$(MAKE) -C MULTI_Start exe


.phony: release_win release_linux

clean:
	rm -rf release_linux
	rm -rf release_win
	$(MAKE) clean -C check_KCC
	$(MAKE) clean -C AUTO_Start
	$(MAKE) clean -C JUMP_Start
	$(MAKE) clean -C MENU_Start
	$(MAKE) clean -C MULTI_Start
