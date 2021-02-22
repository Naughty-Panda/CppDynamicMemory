///////////////////////////////////////////////////
//		C++ Dynamic Memory
//		Naughty Panda @ 2021
///////////////////////////////////////////////////

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>

extern "C"
{
#include "lua54/include/lua.h"
#include "lua54/include/lualib.h"
#include "lua54/include/lauxlib.h"
}

#ifdef _WIN32
#pragma comment(lib, "lua54/lua54.lib")
#endif // _WIN32

#define STACK_TOP -1
#define BUFFER_SIZE 256
#define ARR_SIZE 4

namespace fs = std::filesystem;

const struct LuaContent {

	std::string cnt1 =
		"function GiveMeSomething(input)"
		" local output = 0"
		" if (input == nil) then"
		" output = math.random(100, 999) ";
	std::string cnt2 =
		"return output"
		" end"
		" output = input << 1"
		" return output"
		" end";
};

//////////////////////////////////////////////////////////////////////////////
//	Lua
//////////////////////////////////////////////////////////////////////////////

bool LuaError(lua_State* lua, int code) {

	if (code == LUA_OK) return false;

	std::cout << "LUA ERROR: " << std::endl;
	std::cout << lua_tostring(lua, STACK_TOP) << std::endl;
	return true;
}

lua_State* LuaInit() {

	lua_State* lua = luaL_newstate();
	luaL_openlibs(lua);
	return lua;
}

bool LuaLoadScript(lua_State* lua, std::string filename) {

	filename += ".txt";
	return LuaError(lua, luaL_dofile(lua, filename.c_str()));
}

int LuaExecute(lua_State* lua, const char* command, const bool bArgs, const int nInput = 0) {

	lua_getglobal(lua, command);
	if (lua_isfunction(lua, STACK_TOP)) {
		if (bArgs) {
			lua_pushnumber(lua, nInput);
		}
		if (!LuaError(lua, lua_pcall(lua, (int)bArgs, 1, 0))) {
			return static_cast<int>(lua_tonumber(lua, STACK_TOP));
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
//	6.1
//////////////////////////////////////////////////////////////////////////////

void PrintArray(const int* arr, const size_t& size) {

	if (!arr) return;

	std::cout << "[ ";
	for (size_t i = 0; i < size; i++)
		std::cout << arr[i] << " ";

	std::cout << "]" << std::endl;
}

void PrintArray(int** arr, const size_t& size, const size_t& size2) {

	if (!arr || !arr[0]) return;

	for (size_t m = 0; m < size; m++) {
		for (size_t n = 0; n < size2; n++) {
			std::cout << arr[m][n] << " ";
		}
		std::cout << std::endl;
	}
}

int AskForSize() {

	std::cout << "\nWe are going to allocate an array for you!\n"; 
	int size(0);
	while (size < 2)
	{
		std::cout << "Please enter its size > 1: ";
		std::cin >> size;
		if (std::cin.fail()) break;
	}

	return size;
}

int* AllocateArray(const unsigned int& size) {

	int* mem_ptr = new (std::nothrow) int[size];
	return mem_ptr;
}

int** AllocateArray(const unsigned int& size, const unsigned int& size2) {

	int** memptr = new (std::nothrow) int*[size];

	if (!memptr) return nullptr;

	for (size_t i = 0; i < size2; i++)
		memptr[i] = new (std::nothrow) int[size2];

	return memptr;
}

void DeleteArray(int** arr, const unsigned int& size, const unsigned int& size2) {

	if (!arr || !arr[0]) return;

	for (size_t i = 0; i < size2; i++)
		delete[] arr[i];

	delete[] arr;
}

void __fastcall FillArray(int* arr, const size_t& size, const int& filler = 1, lua_State* lua = nullptr) {
	
	if (!arr) return;

	int temp(1);

	for (size_t i = 0; i < size; i++) {
			if (lua) {
				arr[i] = temp;
				temp = LuaExecute(lua, "GiveMeSomething", true, temp);
			}
			else {
				arr[i] = filler;
			}
	}
}

//////////////////////////////////////////////////////////////////////////////
//	6.2
//////////////////////////////////////////////////////////////////////////////

void __fastcall FillArray2D(int** arr, const size_t& size, const size_t& size2, lua_State* lua) {

	if (!arr || !lua) return;

	for (size_t m = 0; m < size; m++) {
		for (size_t n = 0; n < size2; n++) {
			arr[m][n] = LuaExecute(lua, "GiveMeSomething", false);
		}
	}

}

void __fastcall ClearArray(int* arr, const size_t& size) {
	
	FillArray(arr, size, 0);
}

//////////////////////////////////////////////////////////////////////////////
//	6.3
//////////////////////////////////////////////////////////////////////////////

std::string AskForFilename() {

	std::string filename;

	while (filename.empty())
	{
		std::cout << "Enter file name: ";
		std::cin >> filename;
	}

	return filename;
}

bool CreateFile(std::string filename, const std::string& content) {

	if (filename.empty()) return false;

	filename += ".txt";

	if (fs::exists(filename)) {
		fs::remove(filename);
		std::cout << filename << " was removed.\n";
	}

	std::ofstream fout(filename);
	std::cout << filename << " was successfully created.\n";

	fout << content;
	fout.close();

	return true;
}

//////////////////////////////////////////////////////////////////////////////
//	6.4
//////////////////////////////////////////////////////////////////////////////

std::string* ReadFile(std::string filename) {

	if (filename.empty()) return nullptr;

	filename += ".txt";

	if (!fs::exists(filename)) return nullptr;

	std::ifstream fin(filename);
	if (!fin.is_open()) return nullptr;

	std::string* sContent = new (std::nothrow) std::string;
	if (!sContent) return nullptr;

	char buffer[BUFFER_SIZE];

	while (!fin.eof()) {
		fin.getline(buffer, BUFFER_SIZE);

		*sContent += buffer;
	}

	fin.close();
	return sContent;
}

void PrintFile(std::string filename) {

	std::string* content = ReadFile(filename);

	if (!content || content->empty()) {
		delete content;
		return;
	}

	std::cout << "File content:\n" << *content << std::endl;
	delete content;
}

//////////////////////////////////////////////////////////////////////////////
//	6.5
//////////////////////////////////////////////////////////////////////////////

bool SearchFile(std::string filename, std::string request) {

	if (filename.empty()) return false;

	std::string* sText = ReadFile(filename);

	if (!sText || sText->empty()) {
		delete sText;
		return false;
	}

	size_t nPos = sText->find(request);

	delete sText;
	return nPos != std::string::npos ? true : false;
}

bool _next_search() {

	char sRequest;
	std::cout << "Would you like to search again? Y\\n:";
	std::cin >> sRequest;

	return (std::tolower(sRequest) == 'y') ? true : false;
}

void SearchMenu(std::string filename) {

	bool bQuit(false);
	std::string sRequest;

	while (!bQuit) {

		sRequest.clear();

		while (sRequest.empty()) {
			std::cout << "Enter word to search: ";
			std::cin >> sRequest;
		}

		if (SearchFile(filename, sRequest)) {
			std::cout << "Yay! \"" << sRequest << "\" was found in " << filename << ".txt\n";
		}
		else {
			std::cout << "Nope. \"" << sRequest << "\" was not found in " << filename << ".txt\n";
		}

		bQuit = !_next_search();
	}
}


int main() {

	//////////////////////////////////////////////////////////////////////////////
	//	6.3
	//////////////////////////////////////////////////////////////////////////////

	LuaContent* content = new LuaContent;

	std::cout << "Let's create some files!\n";
	std::string filename = AskForFilename();
	if (!CreateFile(filename, content->cnt1)) return 1;
	PrintFile(filename);

	std::cout << "\nAnd second file: \n";
	std::string filename2 = AskForFilename();

	while (filename == filename2) {
		std::cout << "\nPlease enter another filename!\n";
		filename2 = AskForFilename();
	}

	if (!CreateFile(filename2, content->cnt2)) return 1;
	PrintFile(filename2);

	//////////////////////////////////////////////////////////////////////////////
	//	6.4
	//////////////////////////////////////////////////////////////////////////////

	std::cout << "\nPreparing third file...\n";

	//const std::string cnt1 = ReadFile(filename);
	//const std::string cnt2 = ReadFile(filename2);
	std::string* sFileContent1 = ReadFile(filename);
	if (!sFileContent1) return 1;

	std::string* sFileContent2 = ReadFile(filename2);
	if (!sFileContent2) return 1;

	std::string filename3 = "lua_script";

	if (!CreateFile(filename3, *sFileContent1 + *sFileContent2)) return 1;
	PrintFile(filename3);

	delete sFileContent1;
	delete sFileContent2;

	//////////////////////////////////////////////////////////////////////////////
	//	Lua Init
	//////////////////////////////////////////////////////////////////////////////

	lua_State* lua = LuaInit();
	if (!lua) return 1;

	LuaLoadScript(lua, filename3);

	//////////////////////////////////////////////////////////////////////////////
	//	6.1
	//////////////////////////////////////////////////////////////////////////////

	const int size = AskForSize();
	if (!size) {
		std::cout << "std::cin failed!";
		return 1;
	}

	int* arr1D = AllocateArray(size);
	std::cout << "Clearing newly allocated array:\n";
	ClearArray(arr1D, size);
	PrintArray(arr1D, size);
	std::cout << "\nWe'll ask Lua to fill it for us!\nHere's how it looks now:\n";
	FillArray(arr1D, size, 1, lua);
	PrintArray(arr1D, size);

	delete[] arr1D;
	arr1D = nullptr;

	//////////////////////////////////////////////////////////////////////////////
	//	6.2
	//////////////////////////////////////////////////////////////////////////////

	int** arr2D = AllocateArray(ARR_SIZE, ARR_SIZE);
	std::cout << "\nNow let us allocate 2D array 4 x 4!\n";
	std::cout << "Once again we will ask Lua to fill it with random numbers...\n\n";
	FillArray2D(arr2D, ARR_SIZE, ARR_SIZE, lua);
	PrintArray(arr2D, ARR_SIZE, ARR_SIZE);

	DeleteArray(arr2D, ARR_SIZE, ARR_SIZE);
	arr2D = nullptr;

	//////////////////////////////////////////////////////////////////////////////
	//	6.5
	//////////////////////////////////////////////////////////////////////////////

	std::cout << "\nFor the last task, we'll be searching something in " << filename3 << ".txt\n";
	SearchMenu(filename3);


	delete content;
	content = nullptr;
	lua_close(lua);
	return 0;
}
