#Pinky Controls Readme

This short program will modify the keybindings of Caps Lock and ' to become control. Also rebinds Caps Lock to become escape when not being held. Current settings will only work with qwerty keyboard layouts as key rebindings are hard coded. Changing the 1 liner perl scripts can modify this

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
After installing, the program and script need to be run at login. This can be done in the .xinitrc file
### Arch linux

The original program, Space2Ctrl is currently available via the Arch User Repository (AUR) as 'space2ctrl-git'.

## Usage:
* Load Space2Ctrl with "s2cctl start"
* Unload Space2Ctrl with "s2cctl stop"
