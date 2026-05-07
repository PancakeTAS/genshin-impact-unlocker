# genshin-impact-unlocker

A cut-down version of the [genshin-fps-unlock](https://github.com/34736384/genshin-fps-unlock) project, designed to integrate into Linux systems without the need for a user interface.

## Building and Installing

If building on Linux, follow these instructions:
```bash
$ cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=cmake/mingw64.cmake
$ cmake --build build
$ cp -v build/genshin-impact-launcher.exe "(Genshin Impact)/GenshinImpactUnlocker.exe"
$ cp -v build/libgenshin-impact-unlocker.dll "(Genshin Impact)/GenshinImpactUnlocker.dll"
```

Make sure to replace `(Genshin Impact)` with the actual path to your Genshin Impact installation (the same folder in which GenshinImpact.exe is!)

## Running and Configuring

To run the unlocker, change into the Genshin Impact directory and run the `GenshinImpactUnlocker.exe` file.

The unlocker will create a log file for the launcher in `launcher.log` and a log file for the game in `game.log`.

There is no configuration file as of now, if you wish to change the command line arguments, check out `src/launcher.cpp:L38`. If you want to change the fps limit, edit `src/dll.cpp:L86`. Make sure to recompile the project.
