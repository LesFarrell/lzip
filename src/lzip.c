/*
MIT License

Copyright (c) 2022 Les Farrell

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <zip.h>


// Define some constants for Lua
#define ZIP_MINIMUM_COMPRESSION_LEVEL 0
#define ZIP_MAXIMUM_COMPRESSION_LEVEL 9


// Macro to allow us to export C Constants back to Lua
#define lua_setConst(L, name) \
  {                           \
    lua_pushnumber(L, name);  \
    lua_setglobal(L, #name);  \
  }


//------------------------------------------------------------------------------

/*
 * This is the data structure every instance will hold.
 * It contains a pointer to the zip_t data structure.
 */
struct lzip_data
{
  struct zip_t *zip_t;
};
typedef struct lzip_data lzip_data;


//------------------------------------------------------------------------------

/*
 * Register our modules data structure with Lua.
 *
 * Passed:
 * L               The lua state
 * index           Index to the argument to check
 *
 * Returns:
 * lzip_data *    A new instance of our data structure
 */
lzip_data *check_lzip(lua_State *L, int index)
{
	/* Checks whether the function argument (index) is userdata of the type lzip.db. */
	return (lzip_data *)luaL_checkudata(L, index, "lzip.db");
}

//------------------------------------------------------------------------------

/*
* Decode a zip error number and push the resulting error string onto the stack.
*/
int lzip_geterror(lua_State *L, int err)
{
	lua_pushstring(L, zip_strerror(err));
	return 1;
}

//------------------------------------------------------------------------------

/**
 * Opens zip archive with compression level using the given mode.
 *
 * Passed:
 * zipname zip archive file name.
 * level compression level (0-9 are the standard zlib-style levels).
 * mode file access mode.
 *        - 'r': opens a file for reading/extracting (the file must exists).
 *        - 'w': creates an empty file for writing.
 *        - 'a': appends to an existing archive.
 *
 * Returns:
 * The zip archive handler or NULL on error
 */
static int lzip_open(lua_State *L)
{
	const char *zipname;
	int compressionlevel = ZIP_DEFAULT_COMPRESSION_LEVEL;
	const char *mode = {'\0'};


	if (lua_gettop(L) != 3)
	{
		// Display the usage message.
		luaL_error(L, "usage: open( zipname, compressionlevel, mode)");
	}

	zipname = luaL_checkstring(L, 1);
	compressionlevel = luaL_checknumber(L, 2);
	mode = luaL_checkstring(L, 3);

	// Check the compression level.
	if ((compressionlevel & 0xF) > 9)
	{
		compressionlevel = ZIP_DEFAULT_COMPRESSION_LEVEL;
	}

	// Check we only have one character in mode.
	if (strlen(mode) != 1)
	{
		luaL_error(L, "Unrecognised archive access mode");
	}

	// Check the mode.
	if (mode[0] != 'w' && mode[0] != 'r' && mode[0] != 'a' && mode[0] != 'd')
	{
		luaL_error(L, "Unrecognised archive access mode");
	}

	// Create the user data
	lzip_data *self = (lzip_data *)lua_newuserdata(L, sizeof(lzip_data));

	// Call zip_open function.
	self->zip_t = zip_open(zipname, compressionlevel, mode[0]);
	if (self->zip_t == NULL)
	{
		// Display a error.
		luaL_error(L, "Unable to open archive.");
	}

	// Create the userdata.
	luaL_getmetatable(L, "lzip.db");
	lua_setmetatable(L, -2);

	return 1;
}

//------------------------------------------------------------------------------

/*
 * Close the zip archive.
 */
static int lzip_close(lua_State *L)
{
	// Grab the userdata from Lua's stack
	lzip_data *self = check_lzip(L, 1);

	// Close the archive.
	if (self->zip_t != NULL)
	{
		zip_close(self->zip_t);
	}

	return 0;
}

//------------------------------------------------------------------------------

/*
 * Is this a 64bit zip archive?
 */
int lzip_is64(lua_State *L)
{
	// Grab the userdata from Lua's stack
	lzip_data *self = check_lzip(L, 1);
	
	// Push the result to Lua's stack.
	lua_pushboolean(L, zip_is64(self->zip_t));
	
	return 1;
}

//------------------------------------------------------------------------------

/*
 *	Open a zip entry by name
 */
int lzip_entry_open(lua_State *L)
{
	const char *entryname;
	
	// Grab the userdata from Lua's stack
	lzip_data *self = check_lzip(L, 1);
	
	// Check for the name on the stack
	entryname = luaL_checkstring(L, 2);	
	
	// Open the entry.
	return zip_entry_open(self->zip_t, entryname);
}

//------------------------------------------------------------------------------

/*
 * Open a case sensitive archive entry.
 */
int lzip_entry_opencasesensitive(lua_State *L)
{
	const char *entryname;
	
	// Grab the userdata from Lua's stack.
	lzip_data *self = check_lzip(L, 1);
	
	// Get the entries name from the stack.
	entryname = luaL_checkstring(L, 2);	
	
	// Try to open the entry.
	return zip_entry_opencasesensitive(self->zip_t, entryname);
}

//------------------------------------------------------------------------------

/*
 *	Open an archive entry by index. Indexes start at 0. 
 */
int lzip_entry_openbyindex(lua_State *L)
{
	long Index = 0;
	
	// Grab the userdata from Lua's stack
	lzip_data *self = check_lzip(L, 1);

	// Get the index from Lua's stack.
	Index = (long)luaL_checknumber(L, 2);

	// Try to open the entry.
	return zip_entry_openbyindex(self->zip_t, Index);
}

//------------------------------------------------------------------------------

/*
 *	Places the name of the currently selected entry on the Lua stack.
 */
int lzip_entry_name(lua_State *L)
{
	const char *entryname;
	
	// Grab the userdata from Lua's stack
	lzip_data *self = check_lzip(L, 1);

	// Place the name of the current entry on the stack.
	entryname = zip_entry_name(self->zip_t);
	if (entryname != NULL)
	{
		lua_pushstring(L, entryname);
	}
	else
	{
		lua_pushnil(L);
	}
	return 1;
}

//------------------------------------------------------------------------------

/*
 *	Checks to see if the currently selected entry is a directory.
 */
int lzip_entry_isdir(lua_State *L)
{
	// Grab the userdata from Lua's stack
	lzip_data *self = check_lzip(L, 1);
	
	// Push True or False onto the stack.
	lua_pushboolean(L, zip_entry_isdir(self->zip_t));
	return 1;
}

//------------------------------------------------------------------------------

/*
 *	Places the size of the currently selected entry on Lua's stack.
 */
int lzip_entry_size(lua_State *L)
{
	// Grab the userdata from Lua's stack
	lzip_data *self = check_lzip(L, 1);
	
	// Push the entry size onto the stack.
	lua_pushnumber(L, zip_entry_size(self->zip_t));
	return 1;
}

//------------------------------------------------------------------------------

/*
 *	Places the uncompressed size of the currently selected entry on the Lua stack.
 */
int lzip_entry_uncomp_size(lua_State *L)
{
	// Grab the userdata from Lua's stack
	lzip_data *self = check_lzip(L, 1);
	
	// Push the uncompressed size of the entry onto the stack.
	lua_pushnumber(L, zip_entry_uncomp_size(self->zip_t));
	return 1;
}

//------------------------------------------------------------------------------

/*
 *	Places the compressed size of the currently selected entry on the Lua stack.
 */
int lzip_entry_comp_size(lua_State *L)
{
	// Grab the userdata from Lua's stack
	lzip_data *self = check_lzip(L, 1);
	
	// Push the compressed size of the entry into the stack.
	lua_pushnumber(L, zip_entry_comp_size(self->zip_t));
	return 1;
}

//------------------------------------------------------------------------------

/*
 *	Places the CRC value of the currently selected entry on the Lua stack.
 */
int lzip_entry_crc32(lua_State *L)
{
	// Grab the userdata from Lua's stack
	lzip_data *self = check_lzip(L, 1);
	
	// Push the CRC of the entry onto the stack.
	lua_pushnumber(L, zip_entry_crc32(self->zip_t));
	return 1;
}

//------------------------------------------------------------------------------

/*
 * Write data into the currently selected entry.
 */
int lzip_entry_write(lua_State *L)
{
	const void *buf = NULL;
	size_t size = 0;
	int result = 0;

	// Grab the userdata from Lua's stack
	lzip_data *self = check_lzip(L, 1);
	
	// Get the data from the stack.
	buf = luaL_checkstring(L, 2);
	
	// Get the size of the data from the stack.
	size = luaL_checknumber(L, 3);
	
	// Write the data into the currently selected entry.
	result = zip_entry_write(self->zip_t, buf, size);
	lzip_geterror(L, result);
	return 1;
}

//------------------------------------------------------------------------------

/*
 *	Write the contents of a file into the currently selected entry.
 */
int lzip_entry_fwrite(lua_State *L)
{
	int result = 0;
	
	// Grab the userdata from Lua's stack
	lzip_data *self = check_lzip(L, 1);
	
	// Read the file and write it's data into the current entry.
	result = zip_entry_fwrite(self->zip_t, luaL_checkstring(L, 2));
	lzip_geterror(L, result);
	return 1;
}

//------------------------------------------------------------------------------

/*
 *	Write the contents of the currently selected entry to file.
 */
int lzip_entry_fread(lua_State *L)
{
	int result = 0;
	
	// Grab the userdata from Lua's stack
	lzip_data *self = check_lzip(L, 1);
	
	// Read the current entry and write the data to file.
	result = zip_entry_fread(self->zip_t, luaL_checkstring(L, 2));
	lzip_geterror(L, result);
	return 1;
}

//------------------------------------------------------------------------------

/*
 *	Close the current entry in the archive.
 */
int lzip_entry_close(lua_State *L)
{
	int result = 0;
	
	// Grab the userdata from Lua's stack
	lzip_data *self = check_lzip(L, 1);
	
	// Close the current archive entry.
	result = zip_entry_close(self->zip_t);
	lzip_geterror(L, result);
	return 1;
}

//------------------------------------------------------------------------------

/*
 *	Places the index of the current entry in the archive on the Lua stack.
 */
int lzip_entry_index(lua_State *L)
{
	int result = 0;
	
	// Grab the userdata from Lua's stack
	lzip_data *self = check_lzip(L, 1);
	
	// Find the index of the current entry
	result = zip_entry_index(self->zip_t);
	
	// Place the result on the stack.
	lua_pushnumber(L, result);	
	// lzip_geterror(L,result);
	return 1;
}

//------------------------------------------------------------------------------

/*
 *	Places the total number of entries in the archive on the Lua stack
 */
static int lzip_entries_total(lua_State *L)
{
	// Grab the userdata from Lua's stack
	lzip_data *self = check_lzip(L, 1);
	
	// Push the total number of entries into the stack.
	lua_pushnumber(L, zip_entries_total(self->zip_t));
	return 1;
}

//------------------------------------------------------------------------------

/*
 *	Simple wrapper function which takes a list of files in a lua table and compresses 
 *  them into a zip archive.
 */
int lzipFiles(lua_State *L)
{
  // Initialize the zip state.
  struct zip_t *Zip;
  
  int CompressionLevel = ZIP_DEFAULT_COMPRESSION_LEVEL;

  // Get the compression level required
  if (lua_isnumber(L, 3))
  {
    CompressionLevel = (int) lua_tointeger(L, 3);
  }

  // Check a Lua table was passed.	
  if (lua_istable(L, 2))
  {
    // Push nil onto the stack.
    lua_pushnil(L);

    // Create a zip file using the passed compression level and archive name.
    Zip = zip_open((char *)lua_tostring(L, 1), CompressionLevel, 'w');

    // Loop for each file in the passed table.
    while (lua_next(L, 2) != 0)
    {
      // Check it's a string (file name) on the stack.
      if (lua_isstring(L, -1))
      {
        // Create a entry in the zip for the passed file.
        zip_entry_open(Zip, (char *)lua_tostring(L, -1));

        // Write and compress the file to the archive.
        zip_entry_fwrite(Zip, (char *)lua_tostring(L, -1));

        // Close the entry.
        zip_entry_close(Zip);

        // Pop the last entry off the stack.
        lua_pop(L, 1);
      }
    }

    // Close the Zip file.
    zip_close(Zip);
  }
  return 0;
}

//------------------------------------------------------------------------------

/*
 * When you leave lua without closing the database,
 * the garbage collector will clean up for us.
 */
static int lzip__gc(lua_State *L)
{
	// Close the archive.
	lzip_close(L);
	return 0;
}

//------------------------------------------------------------------------------

/*
 * The methods our object exposes to Lua.
 */
static const luaL_Reg lzip_method_map[] = {
    {"close", lzip_close},
    {"is64", lzip_is64},
    {"entry_opencasesensitive", lzip_entry_opencasesensitive},
    {"entry_openbyindex", lzip_entry_openbyindex},
    {"entry_index", lzip_entry_index},
    {"entry_open", lzip_entry_open},
    {"entry_name", lzip_entry_name},
    {"entry_isdir", lzip_entry_isdir},
    {"entry_size", lzip_entry_size},
    {"entry_uncomp_size", lzip_entry_uncomp_size},
    {"entry_comp_size", lzip_entry_comp_size},
    {"entry_crc32", lzip_entry_crc32},
    {"entry_close", lzip_entry_close},
    {"entries_total", lzip_entries_total},
    {"entry_fwrite", lzip_entry_fwrite},
    {"entry_fread", lzip_entry_fread},
    {"entry_write", lzip_entry_write},
    {"__gc", lzip__gc},
    {NULL, NULL}};

//------------------------------------------------------------------------------

/*
 * The methods this module exposes to Lua.
 */
static const luaL_Reg lzip_module[] = {
    {"open", lzip_open},
    {"compress_files", lzipFiles},
    {NULL, NULL}};

//------------------------------------------------------------------------------

/*
 * The loader called when our shared library is loaded.
 */
int luaopen_lzip(lua_State *L)
{
	// Register the zip compression constants
	lua_setConst(L, ZIP_DEFAULT_COMPRESSION_LEVEL);
	lua_setConst(L, ZIP_MINIMUM_COMPRESSION_LEVEL);
	lua_setConst(L, ZIP_MAXIMUM_COMPRESSION_LEVEL);

#if LUA_VERSION_NUM == 501
	luaL_register(L, "lzip", lzip_module);
#else
	luaL_newlib(L, lzip_module);
#endif

	// Create the userdata
	luaL_newmetatable(L, "lzip.db");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

#if LUA_VERSION_NUM == 501
	luaL_register(L, NULL, lzip_method_map);
#else
	luaL_setfuncs(L, lzip_method_map, 0);
#endif

  lua_pop(L, 1);
  return 1;
}
