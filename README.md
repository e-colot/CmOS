# TODO

- Make a complete test (mix of write, erase, read)
- Measure impact of page size
- Compare type of file storage
- implement a first process management algorithm
- implement different process management algorithms
- compare process management algorithms
- revisit ISA

# Known issues

File terminator:
    when putting a file in disk (no matter which storage method used), it is filled with ```0xFF``` until the end of the used page. This leads to error when reading the file
    Needs to update ```addFile``` and ```loadFile```

# Small things to do

- might remove memPos from diskRead (obsolete argument)
