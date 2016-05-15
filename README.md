# sdtest
Wii U SD benchmark

NOTE:
There is a bug in the timer function that looses seconds.  Seems to be rolling over.
RESULTS ARE USELESS at this time.
1k, 10k, 100k may be OK.

Before use:
Run the createFiles.bat file in the sdtest/data folder.
 This will create the 1.2GB of test files. Run it from the SD card.
 Sorry I assume it's windows only.
 The batch file should create the files the quickest if ran on the SD card.

 It tests random reads of 1k, 10k, 100k & 1M files.

 It also does a test to see what happens if a program such as loadiine put all files in 1 larger uncompressed file. This would save on multiple file open/close and other SD slowdowns. These tests do not reflect the overhead of looking of the files in a table to know where they reside in the large file, but I do not think the overhead would slow things down greatly.

 1k in 1M tests the speed of accessing 1000 random 1k blocks in a 1M file.
 10k in 10M tests the speed of accessing 1000 random 10k blocks in a 10M file.
 etc,
 1k in 10M tests the speed of accessing 10000 random 1k blocks in a 10M file.

 A testdata.txt result file is saved in the sdtest folder

