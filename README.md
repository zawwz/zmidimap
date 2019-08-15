# zmidimap

Map midi signals coming from ALSA midi devices to shell commands  

Dependencies: alsa-utils , [ztd](https://github.com/zawwz/ztd)

## Installing

### Package

#### Debian/Ubuntu
Download the .deb package then run: `sudo dpkg -i zmidimap.deb ; sudo apt -f install`

### Standalone

Move executable `zmidimap` to your PATH directory

### From source

```sh
git clone https://github.com/zawwz/zmidimap.git
cd zmidimap
make && sudo make install  
```

## Usage

`zmidimap [options] <map file>`  
This is a daemon program, it does not start any background process by itself and needs to be constantly running for the mapping to be active

## Map File / Configuration

See `example.mim` for an example file  
For more details see `zmidimap --help`

## Options

To scan for devices use `zmidimap -l`  
To scan a device's inputs use `zmidimap -p <client name>`

For more details see `zmidimap --help`
