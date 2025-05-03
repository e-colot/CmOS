## TODO

### 1. General
- Create test files for the project
- Fill files with 0xFF (```HLT instruction```) before storing them to ensure proper recovery

## 2. Test scenarios

### 2.1. Erasing files
    
    - make sure file is removed from FAT
    - check whether FAT is reorganized

### 2.2. FAT filling

    - make sure every file is in FAT
    - make sure no hole is left in FAT

### 2.3. FAT-page removal

    - check for the good linking

## Known issues

```NULL```
