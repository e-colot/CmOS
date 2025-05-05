# TODO

- Implement a contiguous allocation mode
- Make a complete test (mix of write, erase, read)
- Measure impact of page size
- Compare type of file storage
- implement a first process management algorithm
- implement different process management algorithms
- compare process management algorithms
- revisit ISA
## Contiguous allocation
    - [x] disk init 
    - [x] FAT setup
    - [x] Adding files
    - [x] Removing Files
    - [x] Inserting files in holes
    - [ ] Update FAT tests
    - [ ] File displacement
    - [ ] Disk defragmentation

# Known issues

reorganizeFAT not working for a single FAT page ??

# Small things to do

- might remove memPos from diskRead (obsolete argument)
