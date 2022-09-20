archive = require("lzip")

-- Open a archive for writing.
zip = archive.open("example_two.zip", ZIP_MAXIMUM_COMPRESSION_LEVEL, "w")

-- Give the entry a name.
zip:entry_open("foo-1.txt")

-- Write some data to the entry.
string = "Some data here....."
zip:entry_write(string,#string)

-- Close this entry.
zip:entry_close()

-- Create a new entry.
zip:entry_open("foo-2.txt")
	
-- Merge 3 files into one entry and compress them on-the-fly.
zip:entry_fwrite("file_one.txt");
zip:entry_fwrite("file_two.txt");
zip:entry_fwrite("file_three.txt");
    
-- Close this entry.
zip:entry_close();

-- Close the archive.
zip:close()
