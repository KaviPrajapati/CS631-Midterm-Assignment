AUTHOR:
    NAME : Kavi Prajapati
    CWID : 10459970
    EMAIL : kprajap2@stevens.edu

NAME
    ls : list directory contents

SYNOPSIS
    ls [−AacdFfhiklnqRrSstuw] [file . . .]

HOW TO RUN
    - Run 'make' command to generate the binary. It will run the makefile and generate binary nameed 'ls'
    - Now run the generated binary like shown below.
    `ls (flags) (file ...)`

USAGE - OPTIONS
- A : List all entries except for '.' and '..'
- a : Include directory entries whose names begin with a dot('.')
- i : For each file, print the file’s file serial number (inode number)
- F : Display a slash ( ‘/’ ) immediately after each pathname that is a directory, an asterisk ( ‘∗’) after each that is executable an at sign ( ‘@’ ) after each symbolic link, a percent sign ( ‘%’ ) after each whiteout, an equal sign ( ‘=’ ) after each socket, and a vertical bar ( ‘|’ ) after each that is a FIFO
- S : Sort by size, largest file first
- t : Sort by time modified (most recently modified first) before sorting the operands by lexicographical order
- u : Use time of last access, instead of last modification of the file for sorting ( −t ) or printing ( −l )
- r : Reverse the order of the sort to get reverse lexicographical order or the smallest or oldest entries first
- c : Use time when file status was last changed, instead of time of last modification of the file for sorting ( −t ) or printing
- l : List in long format
- d : Directories are listed as plain files (not searched recursively) and symbolic links in the argument list are not indirected through
- k : Modifies the −s option, causing the sizes to be reported in kilobytes. The rightmost of the −k and −h flags overrides the previous flag
- n : The same as −l, except that the owner and group IDs are displayed numerically rather than converting to a owner or group name
- f : Output is not sorted
- q : Force printing of non-printable characters in file names as the character ‘?’; this is the default when output is to a terminal
- s : Display the number of file system blocks actually used by each file, in units of 512 bytes or BLOCKSIZE
- w : Force raw printing of non-printable characters.

Issues faced :

- Not able to check socket, whiteout file and executable files (Fixed using comparing with file modes)
- Facing issue in printing total file size if -l or -n flags passed before all listing (Right now it will print after listing of files)

Github Link :

https://github.com/KaviPrajapati/CS631-Midterm-Assignment

