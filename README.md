# lzip
Lua binding for https://github.com/kuba--/zip

Create a zip archive and compress some files into it.

```lua
archive = require("lzip")
archive.compress_files("Example_one.zip", {"File_One.txt", "File_Two.txt", "File_Three.txt"}, ZIP_DEFAULT_COMPRESSION_LEVEL)
```

Create a zip archive and add some data to it.

```lua
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
```

List the contents of a zip archive.

```lua
archive = require("lzip")

zip = archive.open("example_one.zip", ZIP_MAXIMUM_COMPRESSION_LEVEL, "r")

for n = 0, zip:entries_total() - 1, 1 do
	zip:entry_openbyindex(n)    
	print("Name        : " .. zip:entry_name())	
	print("IsDirectory : " .. (zip:entry_isdir() and 'true' or 'false') )
	print("Size        : " .. zip:entry_size())
	print("CRC         : " .. zip:entry_crc32())    
	print("-------------------------------")    
    zip:entry_close();
end 

zip:close();
```

Extract a file from a archive.

```lua
archive = require("lzip")

zip = archive.open("example_one.zip", 0, "r")

zip:entry_open("File_One.txt");
zip:entry_fread("./extracted.txt");
zip:entry_close();

zip:close();
```

