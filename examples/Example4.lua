archive = require("lzip")

zip = archive.open("example_one.zip", 0, "r")

zip:entry_open("File_One.txt");
zip:entry_fread("example4.txt");
zip:entry_close();

zip:close();

