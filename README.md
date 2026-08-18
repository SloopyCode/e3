# s4

s4 is a simple newdesktop system made for hobby operating systems

it is written from scratch and doesnt use x11 or wayland, 
s4 has its own simple UI framework called [ui16](https://github.com/sulfurLabs/s4/tree/main/ui16) and [libdesktop](https://github.com/sulfurLabs/s4/tree/main/libdesktop) for creating/managing windows for a app

this project is still in pretty early dev and things will probably not work or will change in future.

## What is s4?

The main idea behind s4 is to have a simple desktop that can run directly on a hobby OS without depending on a big existing desktop stack, meaning it just needs a framebuffer device + mouse/keyboard device and basic libc implementations (just get doom running and add eventfd, spawn() and/or execve+fork, and ipc)

The desktop currently has basic things like:

* window manager (also a tiling WM option :3)
* compositor (simple bilineal scaling too)
* rendering code
* input handling (in libdesktop using pollEvents, its kinda working)
* taskbar with the most basic start menu
* background images (bmp and tga are supported)
* file based, or eventfd IPC in newer versions, for libdesktop

Most of this lives inside the `s4/` directory.

## ui16

[ui16](https://github.com/sulfurLabs/s4/tree/main/ui16) is the UI framework used for making s4 applications.

it's a small tree/container based UI framework and is meant to be simple and easy to use instead of bringing in something like gtk3 eww

e.g, a UI can be built from containers and buttons like this:

```c
ui16_container(
    style(
        width(fill),
        height(fill)
    )
) {
    ui16_button("hello world");
}
```
[whole app example](https://github.com/sulfurLabs/sulfurOS/blob/main/user/bin/template/main.c)


ui16 can use percentages for sizes, so containers can share the available space without having to calculate everything manually, cuz ui16 handles that calculation

(there's a README in `ui16/` with a little more information and a better example)

## libdesktop

[libdesktop](https://github.com/sulfurLabs/s4/tree/main/libdesktop) is the small library used for talking to the desktop system
you create windows or popups with it, create a window buffer and give it to e.g. ui16\

## Building

i'm trying to port s4 to many different hobbyOS'es. (look into s4/os.h for a list of kinda working OS list)

the whole project uses a x86_64 elf cross compiler,

```text
x86_64-elf-gcc
x86_64-elf-ld
```

in the makefile you find this:

```text
OS_PATH     ?= ~/sulfurLabs/sulfurOS/
```

if you want to run it on your own OS then you need to change those paths.

Then just build with:

```bash
make fetchDeps
make
```

the fetchDeps fetches the libc from sulfurOS, but you would need to 

<br>

## License

this code is licensed under the GNU GPLv3.

If you fork or redistribute s4, please keep the attribution to the original s4 project as described in ATTRIBUTION.md.
