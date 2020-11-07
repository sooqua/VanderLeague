#include "mmap.hpp"

int main(int argc, char **argv) { 
	mmap kernelMapper;

	LOG("Trying to attach to League of Legends.exe...");

	if (!kernelMapper.attach_to_process("League of Legends.exe"))
		return 1;

	if (!kernelMapper.load_dll("Ghvoaevikr.dll"))
		return 1;

	if (!kernelMapper.inject())
		return 1;

	LOG("\nPress any key to close.");
	static_cast<void>(std::getchar());
	 
	return 0;
}