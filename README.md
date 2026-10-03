# ebpk

Easy Backup (ebkp) is a very simple C program that uses file metadata to know what needs to be saved or not, copying again to the target only files that were updated or created since the last backup.

How it works: when the program runs, it first tries to read a `.ebkp` file from the target folder. This file is the file that the program creates after it's execution to save metadata about the backup. From this file it loads (if exists) information about each file copied by the last backup, and crosses this information with the metadata of every file in src_dir to determine which ones needs to be copied again.

## Compatibility

This program should be compatible with any version of Linux, Unix, BSD and MacOS systems.

## Compilation

To compile you will need:
- GCC or Clang
- Make (Optional)

After downloading the repository, open a terminal inside it's main directory and run

```bash
gcc bst.c ebkp.c -o ebkp -Wall -march=native -O3
```
or if you have Make installed, simply

```bash
make
```

## Usage
```
ebkp src_dir tgt_dir [flags]
```

src_dir is the directory that will be copied (all children dirs will also be copied)
tgt_dir is the directory where src_dir will be copied to. It don't need to be created previously
--debug OR -debug will print debug info during execution. Do not use if want to avoid verbose.
--git OR -git will also copy .git folders and it`s contents. Disabled by default.
`--help OR -help display this message and finishes program execution.
