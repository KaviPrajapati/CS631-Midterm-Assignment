Implemented options so far

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

Issues faced :

Not able to check socket and whiteout file

Github Link :

https://github.com/KaviPrajapati/CS631-Midterm-Assignment



-d
-q
-s
-w
