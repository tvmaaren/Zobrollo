/*
Zobrollo is a 2d minimalistic top-view racing game
Copyright (C) 2021  Thomas van Maaren

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

e-mail:thomas.v.maaren@outlook.com
*/

#ifdef __linux__
	#define home_var "HOME"

        #define sep_char '/'
        #define sep_str "/"
	
	#define data_dir "/usr/share/zobrollo"
	#define local_dir "/.config/zobrollo"
#elif	 _WIN32
	#define home_var "userprofile"

        #define sep_char '\\'
	#define sep_str "\\"

	#define data_dir "."
	#define local_dir "\\.zobrollo"
#else
	#define home_var "HOME"

	#define sep_char '/'
	#define sep_str "/"
	
	#define data_dir "."
	#define local_dir "/.zobrollo"
#endif

//Paths that the program needs. "config path" is not included, because (at the moment) the program
//only needs to read from it once.
typedef struct{
	char* home;
	char* record;
	char* ghost;
	char* data;
}PATHS;
// vim: cc=100
