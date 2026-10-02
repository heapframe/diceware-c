# Diceware in C
Designed for the lowest possible memory footprint, thats why the code looks awkward.  

## Setup
```sh
git clone https://github.com/heapframe/diceware-c
cd diceware-c
sudo cp eff_large_wordlist.txt /opt/
make
cp diceware *somewhere in your path, .local/bin is good*
```
You can put the wordlist somewhere other than `/opt/` by changing `WORDLIST_PATH` in [main.c](https://github.com/heapframe/diceware-c/blob/main/main.c#L12)  

## Usage
```sh
➜ ./diceware 
whimsical unstylish return boundless acetone
➜ ./diceware 4
skittle stool untreated till
➜ ./diceware 4 - 
frigidity-unwatched-exalted-theorize
➜ ./diceware 3 " separator "
android separator grab separator upon
➜ 
```