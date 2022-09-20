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

