

all:
	$(MAKE) -C patch
	$(MAKE) -C spleef
	$(MAKE) -C infected
	$(MAKE) -C gamerules
	$(MAKE) -C gun-game
	$(MAKE) -C infinite-climber
	$(MAKE) -C race
	
clean:
	$(MAKE) -C patch clean
	$(MAKE) -C spleef clean
	$(MAKE) -C infected clean
	$(MAKE) -C gamerules clean
	$(MAKE) -C gun-game clean
	$(MAKE) -C infinite-climber clean
	$(MAKE) -C race clean

