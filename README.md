# deadlocked-patch

Patches applied to all Deadlocked PS2 clients connecting to the Deadlocked custom server.

## Build

You'll need a local copy of the [PS2SDK](https://github.com/ps2dev/ps2sdk). I recommend using docker. Instructions for how to build using docker are below.

If you are using a local copy, make sure you install [libdl](https://github.com/Dnawrkshp/libdl) first.

Clone the repo and enter it:
```sh
git clone https://github.com/Dnawrkshp/deadlocked-patch.git
cd deadlocked-patch
```

If you want to use docker, run the following commands:
```sh
docker pull ps2dev/ps2dev
docker run -it --rm -v "$PWD\:/src" ps2dev/ps2dev
cd src
apk add bash && ./docker-init.sh
```

Then you can build using:
```sh
make
```

## Memory Usage

```arm
0x000C8000 - 0x000CF000 ; Gamerules
0x000CF000 - 0x000D0000 ; Module Definitions
0x000D0000 - 0x000D8000 ; Patch
0x000D8000 - 0x000DC000 ; Gamemode
0x000E0000 - 0x000F0000 ; Map loader irx modules
```
