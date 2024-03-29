

all:
	$(MAKE) -C patch
	$(MAKE) -C unpatch
	$(MAKE) -C spleef
	$(MAKE) -C infected
	$(MAKE) -C gamerules
	$(MAKE) -C gun-game
	$(MAKE) -C infinite-climber
	$(MAKE) -C race
	$(MAKE) -C snd
	$(MAKE) -C duckhunt
	$(MAKE) -C gridiron
	$(MAKE) -C thousand-kills
	
clean:
	$(MAKE) -C patch clean
	$(MAKE) -C unpatch clean
	$(MAKE) -C spleef clean
	$(MAKE) -C infected clean
	$(MAKE) -C gamerules clean
	$(MAKE) -C gun-game clean
	$(MAKE) -C infinite-climber clean
	$(MAKE) -C race clean
	$(MAKE) -C snd clean
	$(MAKE) -C duckhunt clean
	$(MAKE) -C gridiron clean
	$(MAKE) -C thousand-kills clean

