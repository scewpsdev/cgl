#include <platform.h>

void* memset(void* dst, int val, unsigned long long len);
void* memcpy(void* dst, const void* src, unsigned long long len);

unsigned long long sys_get_file_size(const char* path);
unsigned long long sys_get_file_modified_time(const char* path);
int sys_copy_file(const char* src, const char* dst);
void sys_sleep(int millis);
void* sys_load_library(const char* path);
void sys_free_library(void* lib);
void* sys_get_proc_address(void* lib, const char* name);

unsigned long long sys_write_file(void* file, const unsigned char* buffer, unsigned long long size);
void* sys_get_std_file(unsigned int stream);

void write_int(int i) {
	char buffer[64];
	memset(buffer, 0, sizeof(buffer));
	int digits = 0;
	if (i == 0) buffer[digits++] = '0';
	while (i) {
		buffer[64 - 1 - digits] = '0' + (i % 10);
		digits++;
		i /= 10;
	}
	sys_write_file(sys_get_std_file(2), buffer + 64 - digits, digits);
}

int _main(int argc, string* argv) {
	int dllVersion = 0;

	char appDllCopy[256];
	memset(appDllCopy, 0, sizeof(appDllCopy));
	memcpy(appDllCopy, APP_DLL_NAME, sizeof(APP_DLL_NAME));
	appDllCopy[sizeof(APP_DLL_NAME) - 1] = '-';
	appDllCopy[sizeof(APP_DLL_NAME)] = '0' + dllVersion;
	memcpy(appDllCopy + sizeof(APP_DLL_NAME) + 1, ".dll", 4);

	//sys_write_file(sys_get_std_file(2), APP_DLL_NAME, sizeof(APP_DLL_NAME));
	//sys_write_file(sys_get_std_file(2), APP_DLL, sizeof(APP_DLL));
	//sys_write_file(sys_get_std_file(2), appDllCopy, 64);

	sys_copy_file(APP_DLL, appDllCopy);

	void* lib = sys_load_library(appDllCopy);
	if (!lib) return 1;

	unsigned long long dllWriteTime = sys_get_file_modified_time(APP_DLL);

	int(*load)(void) = (int(*)(void))sys_get_proc_address(lib, "dll_load");
	int(*unload)(void) = (int(*)(void))sys_get_proc_address(lib, "dll_unload");
	int(*reload)(void) = (int(*)(void))sys_get_proc_address(lib, "dll_reload");
	int(*update)(void) = (int(*)(void))sys_get_proc_address(lib, "dll_update");
	if (!load || !unload || !reload || !update) return 2;

	int result = load();
	if (result != 0) return result;

	int running = 1;
	while (running) {
		result = update();
		if (result != 0) {
			running = 0;
		}

		unsigned long long newDllWriteTime = sys_get_file_modified_time(APP_DLL);

		if (newDllWriteTime != dllWriteTime) {
			sys_free_library(lib);
			lib = 0;

			dllVersion++;
			appDllCopy[sizeof(APP_DLL_NAME)] = '0' + dllVersion;
			dllWriteTime = sys_get_file_modified_time(APP_DLL);

			sys_copy_file(APP_DLL, appDllCopy);

			//write_int((int)sys_get_file_size(appDllCopy));
			//write_int((int)sys_get_file_size(APP_DLL));

			/*
			unsigned long long newDllSize = sys_get_file_size(appDllCopy);
			unsigned long long newDllSizeCopy = 0;
			while (newDllSize == 0 || newDllSize != newDllSizeCopy) {
				write_int((int)newDllSize);
				sys_sleep(1);
				newDllSizeCopy = newDllSize;
				newDllSize = sys_get_file_size(appDllCopy);
			}
			*/

			lib = sys_load_library(appDllCopy);
			if (!lib) return 1;

			load = (int(*)(void))sys_get_proc_address(lib, "dll_load");
			unload = (int(*)(void))sys_get_proc_address(lib, "dll_unload");
			reload = (int(*)(void))sys_get_proc_address(lib, "dll_reload");
			update = (int(*)(void))sys_get_proc_address(lib, "dll_update");
			if (!load || !unload || !reload || !update) return 2;

			result = reload();
			if (result != 0) {
				return result;
			}
		}
	}

	result = unload();

	sys_free_library(lib);

	return result;
}
