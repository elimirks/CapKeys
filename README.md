# Warning

This project is pretty hacky, and specific to the Polish Programmers Dvorak keyboard layout.
I may come back some day and document this properly and clean up the code... but that day is yet to come.

#Space2Ctrl Readme

This little hack for X turns the spacebar key into another control key when used in combination.
When used alone, it behaves like the ordinary space bar on the key release event.
Especially useful with Emacs.

## Prerequisites:
* Install X11 and XTEST development packages. On Debian GNU/Linux derivatives:

```bash
sudo apt-get install libx11-dev libxtst-dev
```
* If the program complains about a missing "XRecord" module, enable it by adding 'Load "record"' to the Module section of /etc/X11/xorg.conf:
(This step is unnecessary in most systems)
e.g.:

```
    Section "Module"
            Load  "record"
    EndSection
```

## Install:
```bash
make
sudo make install
```
### Arch linux

Space2Ctrl is currently available via the Arch User Repository (AUR) as 'space2ctrl-git'.

## Usage:
* Load Space2Ctrl with "s2cctl start"
* Unload Space2Ctrl with "s2cctl stop"
