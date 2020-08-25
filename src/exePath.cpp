// Vulkan
// Copyright © 2020 otreblan
//
// vulkan-hello is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// vulkan-hello is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with vulkan-hello.  If not, see <http://www.gnu.org/licenses/>.

#include <fstream>
#include <iostream>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <exePath.hpp>

#define PID_MAX_LENGTH 7

std::filesystem::path exePath()
{
	using namespace std::filesystem;

	//    6  7  4  1
	// /proc/*/exe\0
	char exeSymLink[6+7+4+1];

	sprintf(exeSymLink, "/proc/%d/exe", getpid());

	char* realExe = realpath(exeSymLink, nullptr);

	path exe(dirname(realExe));

	free(realExe);

	return exe;
}
