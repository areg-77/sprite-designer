#pragma comment(lib, "winmm.lib")
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <vector>
#include <thread>
#include <conio.h>

#define to_hex_str(hex_val, sw) (static_cast<std::stringstream const&>(std::stringstream() << std::hex << std::uppercase << std::setw(sw) << std::setfill('0') << hex_val)).str()
#define to_hex_num(hex_val) (static_cast<std::stringstream const&>(std::stringstream() << std::hex << std::uppercase << hex_val)).str()
#define filled_num(val, sw) (static_cast<std::stringstream const&>(std::stringstream() << std::setw(sw) << std::setfill('0') << val)).str()

#define float_num(val, sp) (static_cast<std::stringstream const&>(std::stringstream() << fixed << setprecision(sp) << val)).str()

using namespace std;
using namespace this_thread;

void console_config(short width, short height, int f_sizeY, int f_sizeX, int f_style, bool centerize, const wchar_t* font = L"", const wchar_t* title = L"") {
	CONSOLE_FONT_INFOEX cfi{};
	cfi.cbSize = sizeof(cfi);
	cfi.nFont = 0;
	cfi.dwFontSize.X = f_sizeX;
	cfi.dwFontSize.Y = f_sizeY;
	cfi.FontFamily = FW_DONTCARE;
	cfi.FontWeight = f_style;
	wcscpy_s(cfi.FaceName, font);
	SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);

	CONSOLE_SCREEN_BUFFER_INFOEX csbi;
	csbi.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
	GetConsoleScreenBufferInfoEx(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	csbi.dwSize.X = width;
	csbi.dwSize.Y = height;
	csbi.srWindow = { 0, 0, width, height };
	csbi.dwMaximumWindowSize = { width, height };
	for (int i = 0; i < 16; i++)
		csbi.ColorTable[i] = 0;
	csbi.ColorTable[7] = 0xFFFFFF;

	SetConsoleScreenBufferInfoEx(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_EXTENDED_FLAGS);

	SetWindowLong(GetConsoleWindow(), GWL_STYLE, GetWindowLong(GetConsoleWindow(), GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);
	ShowScrollBar(GetConsoleWindow(), SB_BOTH, 0);
	if (title != L"")
		SetConsoleTitle(title);

	if (centerize) {
		RECT rect;
		GetWindowRect(GetConsoleWindow(), &rect);
		int c_width = rect.right - rect.left;
		int c_height = rect.bottom - rect.top;
		SetWindowPos(GetConsoleWindow(), 0, GetSystemMetrics(SM_CXSCREEN) / 2 - c_width / 2, GetSystemMetrics(SM_CYSCREEN) / 2 - c_height / 2, c_width, c_height, 0);
	}
}

bool is_null(vector<int> sprite_s, int beg, int end) {
	for (int i = beg; i <= end; i++) {
		if (sprite_s[i] != 0)
			return false;
	}
	return true;
}

void add_frame(vector<vector<vector<int>>>& sprite, vector<int>& delay, vector<string>& audio, vector<int>& bright, int frame = -1) {
	vector<vector<int>> sprite_px;
	vector<int> sprite_null(12);
	sprite_px.push_back(sprite_null);
	sprite.push_back(sprite_px);

	audio.push_back("0");
	bright.push_back(100);
	if (frame == -1)
		delay.push_back(0);
	else
		delay.push_back(delay[frame]);
}

void addhere_frame(vector<vector<vector<int>>>& sprite, vector<int>& delay, vector<string>& audio, vector<int>& bright, int frame) {
	vector<vector<int>> sprite_px;
	vector<int> sprite_null(12);
	sprite_px.push_back(sprite_null);
	sprite.insert(sprite.begin() + frame + 1, sprite_px);

	audio.push_back("0");
	bright.push_back(100);
	delay.push_back(delay[frame]);
}

int bright_fx(int clr, int bright) {
	if (clr * bright / 100 > 255)
		return 255;
	return clr * bright / 100;
}

float get_file_size(vector<vector<vector<int>>>& sprite, vector<int> delay, vector<string> audio, vector<int> bright, int width, int height, int f_sizeY, int f_sizeX, string& file_size_type, int mode) {
	float size = (to_string(width) + to_string(height) + to_string(f_sizeY) + to_string(f_sizeX) + to_string(sprite.size())).length() + 4;
	file_size_type = "B";

	for (int fr = 0; fr < sprite.size(); fr++) {
		size += (to_string(sprite[fr].size()) + to_string(delay[fr]) + audio[fr]).length() + 4;
		for (int px = 0; px < sprite[fr].size(); px++) {
			string clrf_hex, clrb_hex;
			for (int i = 3; i <= 5; i++) {
				clrf_hex += to_hex_str(bright_fx(sprite[fr][px][i], bright[fr]), 2);
				clrb_hex += to_hex_str(bright_fx(sprite[fr][px][i + 3], bright[fr]), 2);
			}
			while (clrf_hex[0] == '0' && clrf_hex.length() > 1)
				clrf_hex.erase(clrf_hex.begin());
			while (clrb_hex[0] == '0' && clrb_hex.length() > 1)
				clrb_hex.erase(clrb_hex.begin());
			size += (to_hex_num(sprite[fr][px][0]) + to_string(sprite[fr][px][1]) + to_string(sprite[fr][px][2]) + to_string(sprite[fr][px][10]) + clrf_hex + clrb_hex).length() + 10;
		}
	}

	if (mode == 1 || mode == 2) {
		size /= 1024;	//kb
		file_size_type = "KB";
	}

	if (mode == 2) {
		size /= 1024;	//mb
		file_size_type = "MB";
	}

	return size;
}

int max_elem(vector<vector<vector<int>>> vec, int fr, int n) {
	int max = vec[fr][0][n];
	for (int i = 0; i < vec[fr].size(); i++) {
		if (vec[fr][i][n] > max)
			max = vec[fr][i][n];
	}
	return max;
}

int min_elem(vector<vector<vector<int>>> vec, int fr, int n) {
	int min = vec[fr][0][n];
	for (int i = 0; i < vec[fr].size(); i++) {
		if (vec[fr][i][n] < min)
			min = vec[fr][i][n];
	}
	return min;
}

int max_elem(vector<vector<int>> vec, int n) {
	int max = vec[0][n];
	for (int i = 0; i < vec.size(); i++) {
		if (vec[i][n] > max)
			max = vec[i][n];
	}
	return max;
}

int min_elem(vector<vector<int>> vec, int n) {
	int min = vec[0][n];
	for (int i = 0; i < vec.size(); i++) {
		if (vec[i][n] < min)
			min = vec[i][n];
	}
	return min;
}

string get_conf0(int* conf) {
	switch (conf[0]) {
	case 1:	//collsiion
		return "\033[48;5;196m\033[38;5;0mC\033[0m";
	case 2:	//trigger
		return "\033[48;5;245m\033[38;5;0mT\033[0m";
	case 3:	//update
		return "\033[48;5;227m\033[38;5;0mU\033[0m";
	}

	return "";
}

string get_conf1(int* conf) {
	if (conf[0] == 2)
		return to_string(conf[1]) + "CH";

	return to_string(conf[1]);
}

void UI(int frame, int sym, int* conf, int x, int y, int* clr_f, int* clr_b, bool config_mode, bool lock_frames, bool audio_path, vector<int> delay, vector<string> audio, vector<int> bright, vector<vector<vector<int>>> sprite, int incdec, bool hex_mode, int size_format, int width, int height, int f_sizeY, int f_sizeX) {
	string size_type;
	int px_state = 0;
	for (int i = 0; i < sprite[frame].size(); i++) {
		if (x == sprite[frame][i][1] && y == sprite[frame][i][2]) {
			if (!config_mode)
				px_state = bool(sprite[frame][i][9] + sprite[frame][i][10] + sprite[frame][i][11]);
			else if (sprite[frame][i][0] != 0)
				px_state = 1;
			break;
		}
	}

	int pixels = 0;
	for (int i = 0; i < sprite[frame].size(); i++) {	//getting pixels amount
		if (sprite[frame][i][0] != 0 && !config_mode)
			pixels++;
		if (!is_null(sprite[frame][i], 9, 11) && config_mode)
			pixels++;
	}

	printf("\033[?25l");
	printf("\033[%d;1H\033[0J", height + 1);

	printf("\033[38;5;255m\033[%d;2H%d:%d\033[0m", height + 2, x, y);
	printf("\033[38;5;255m\033[%d;2H%d/%d\033[0m", height + 5, frame + 1, (int)sprite.size());
	if (lock_frames)
		printf("\033[38;5;255m x\033[0m");
	if (audio_path)
		printf("\033[38;5;240m \033[4m%s\033[0m", audio[frame].c_str());
	else {
		if (audio[frame].length() <= 10)
			printf("\033[38;5;240m \033[4m%s\033[0m", audio[frame].c_str());
		else
			printf("\033[38;5;240m ..\033[4m%s\033[0m", string(audio[frame].end() - 10, audio[frame].end()).c_str());
	}

	string bright_tmp = to_string(bright[frame] - 100) + '%';
	if (bright[frame] - 100 > 0)
		bright_tmp = '+' + bright_tmp;
	printf("\033[38;5;255m\033[%d;2H%dms \033[38;5;240m%s\033[0m", height + 6, delay[frame], bright_tmp.c_str());

	if (incdec == 0)
		printf("\033[38;5;247m\033[%d;%dH>\033[0m", height + 2, width - 13);
	if (config_mode)
		printf("\033[38;5;255m\033[%d;%dHconf: %s", height + 2, width - 11, get_conf0(conf).c_str());
	else
		printf("\033[38;5;255m\033[%d;%dHsym: \033[38;2;%d;%d;%dm\033[48;2;%d;%d;%dm%c\033[0m", height + 2, width - 11, clr_f[0], clr_f[1], clr_f[2], clr_b[0], clr_b[1], clr_b[2], char(sym));

	if (config_mode) {
		if (conf[0] == 2)
			printf("\033[%d;%dH%s", height + 3, width - 11, get_conf1(conf).c_str());
	}
	else {
		printf("\033[38;5;247m(%d)\033[0m", sym);
		if (hex_mode) {
			string clrf_hex, clrb_hex;
			for (int c = 0; c < 3; c++) {
				clrf_hex += to_hex_str(clr_f[c], 2);
				clrb_hex += to_hex_str(clr_b[c], 2);
			}
			printf("\033[38;2;%d;%d;%dm\033[%d;%dH#%s\033[0m", clr_f[0], clr_f[1], clr_f[2], height + 3, width - 11, clrf_hex.c_str());
			printf("\033[38;2;%d;%d;%dm\033[%d;%dH#%s\033[0m", clr_b[0], clr_b[1], clr_b[2], height + 4, width - 11, clrb_hex.c_str());
		}
		else {
			printf("\033[38;2;%d;%d;%dm\033[%d;%dH%s:%s:%s\033[0m", clr_f[0], clr_f[1], clr_f[2], height + 3, width - 11, filled_num(clr_f[0], 3).c_str(), filled_num(clr_f[1], 3).c_str(), filled_num(clr_f[2], 3).c_str());
			printf("\033[38;2;%d;%d;%dm\033[%d;%dH%s:%s:%s\033[0m", clr_b[0], clr_b[1], clr_b[2], height + 4, width - 11, filled_num(clr_b[0], 3).c_str(), filled_num(clr_b[1], 3).c_str(), filled_num(clr_b[2], 3).c_str());
		}
	}
	if (incdec == 1) {
		printf("\033[38;5;247m\033[%d;%dH>\033[0m", height + 3, width - 13);
		if (!config_mode)
			printf("\033[38;5;247m\033[%d;%dH>\033[0m", height + 4, width - 13);
	}

	printf("\033[38;5;255m\033[%d;%dH%dpx \033[0m", height + 5, width - 11, pixels);
	if (px_state == 1) {
		if (config_mode)
			printf("\033[38;5;255msym\033[0m");
		else
			printf("\033[38;5;255mconf\033[0m");
	}

	if (size_format < 3)
		printf("\033[38;5;255m\033[%d;%dH%s%s\033[0m", height + 6, width - 11, float_num(get_file_size(sprite, delay, audio, bright, width, height, f_sizeY, f_sizeX, size_type, size_format), bool(size_format)).c_str(), size_type.c_str());
}

void check_pos(vector<vector<vector<int>>>& sprite, int frame) {
	vector<int> sprite_null(12);
	for (int i = 0; i < sprite[frame].size(); i++) {	//deleting null
		if (sprite[frame][i] == sprite_null) {
			sprite[frame].erase(sprite[frame].begin() + i);
			break;
		}
	}
}

void save_pos(int frame, int sym, int* conf, int x, int y, int* clr_f, int* clr_b, bool config_mode, vector<vector<vector<int>>>& sprite) {
	vector<int> sprite_s = { sym, x, y, clr_f[0], clr_f[1], clr_f[2], clr_b[0], clr_b[1], clr_b[2], 0, 0, 0 };
	vector<int> spite_c = { 0, x, y, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	int counter = 0;
	for (int i = 0; i < sprite[frame].size(); i++) {
		if (sprite[frame][i][1] == x && sprite[frame][i][2] == y)
			counter++;
		if (counter == 1) {
			counter++;
			if (!config_mode) {
				sprite[frame][i][0] = sym;
				for (int c = 0; c < 3; c++)
					sprite[frame][i][3 + c] = clr_f[c];
				for (int c = 0; c < 3; c++)
					sprite[frame][i][6 + c] = clr_b[c];
			}
			else
				sprite[frame][i][8 + conf[0]] = conf[1];
		}
	}

	if (config_mode) {
		sprite_s = { 0, x, y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		sprite_s[8 + conf[0]] = conf[1];
	}

	check_pos(sprite, frame);
	if (counter == 0)
		sprite[frame].push_back(sprite_s);
	check_pos(sprite, frame);
}

void erase_pos(int frame, int x, int y, int* conf, bool config_mode, vector<vector<vector<int>>>& sprite) {
	vector<int> sprite_null(12);
	for (int i = 0; i < sprite[frame].size(); i++) {	//checking nulls and erasing
		if (sprite[frame][i][1] == x && sprite[frame][i][2] == y) {
			if (config_mode && !is_null(sprite[frame][i], 9, 11))
				sprite[frame][i][8 + conf[0]] = 0;
			if (!config_mode && !is_null(sprite[frame][i], 9, 11) && sprite[frame][i][0] != 0) {
				sprite[frame][i][0] = 0;
				for (int c = 3; c <= 8; c++)
					sprite[frame][i][c] = 0;
			}
			if (!config_mode && is_null(sprite[frame][i], 9, 11))
				sprite[frame].erase(sprite[frame].begin() + i);
			break;
		}
	}

	if (sprite[frame].size() == 0)
		sprite[frame].push_back(sprite_null);
}

void global_pos(vector<vector<vector<int>>>& sprite, int frame, int x, int y) {
	vector<int> sprite_null(12);

	int counter = 0;
	for (int i = 0; i < sprite[frame].size(); i++) {
		if (sprite[frame][i] == sprite_null)
			counter++;
	}
	if (counter == 0) {
		for (int i = 0; i < sprite[frame].size(); i++) {	//clear
			if (sprite[frame][i][0] != 0)
				printf("\033[%d;%dH ", sprite[frame][i][2], sprite[frame][i][1]);
		}

		for (int i = 0; i < sprite[frame].size(); i++) {
			sprite[frame][i][1] += x;
			sprite[frame][i][2] += y;
		}
	}
}

void global_pos(vector<vector<int>>& sprite, int x, int y, int width, int height) {
	vector<int> sprite_null(12);

	int counter = 0;
	for (int i = 0; i < sprite.size(); i++) {
		if (sprite[i] == sprite_null)
			counter++;
	}
	if (counter == 0) {
		for (int i = 0; i < sprite.size(); i++) {	//clear
			if (sprite[i][0] != 0 && sprite[i][1] >= 1 && sprite[i][1] <= width && sprite[i][2] >= 1 && sprite[i][2] <= height)
				printf("\033[%d;%dH ", sprite[i][2], sprite[i][1]);
		}

		for (int i = 0; i < sprite.size(); i++) {
			sprite[i][1] += x;
			sprite[i][2] += y;
		}
	}
}

int get_n(vector<vector<vector<int>>> sprite, int frame, int x, int y, int n, int init = 0) {
	for (int px = 0; px < sprite[frame].size(); px++) {
		if (sprite[frame][px][1] == x && sprite[frame][px][2] == y && (sprite[frame][px][0] != 0 || !is_null(sprite[frame][px], 9, 11)))
			return sprite[frame][px][n];
	}
	return init;
}

int get_nclr(vector<vector<vector<int>>> sprite, int frame, int x, int y, int n, int init) {
	for (int px = 0; px < sprite[frame].size(); px++) {
		if (sprite[frame][px][1] == x && sprite[frame][px][2] == y && sprite[frame][px][0] != 0)
			return sprite[frame][px][n];
	}
	return init;
}

string get_file(int mode) {
	string file_name;

	OPENFILENAME ofn{};
	TCHAR szFile[255]{};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = GetConsoleWindow();
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	if (mode == 0)
		ofn.lpstrTitle = L"Open Sprite";
	if (mode == 1)
		ofn.lpstrTitle = L"Save Sprite";
	ofn.lpstrFilter = L"(*.spr) Sprite Designer File\0*.spr\0";
	if (mode == 2) {
		ofn.lpstrTitle = L"Open Audio";
		ofn.lpstrFilter = L"(*.wav) Waveform Audio File\0*.wav\0";
	}

	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (mode == 0 || mode == 2) {
		if (GetOpenFileName(&ofn) == TRUE) {
			wstring wname = wstring(ofn.lpstrFile);
			file_name = string(wname.begin(), wname.end());
		}
	}
	if (mode == 1) {
		if (GetSaveFileName(&ofn) == TRUE) {
			wstring wname = wstring(ofn.lpstrFile);
			file_name = string(wname.begin(), wname.end());
		}
	}
	replace(file_name.begin(), file_name.end(), '\\', '/');
	if (string(file_name.end() - 4, file_name.end()) != ".spr" && file_name != "" && mode != 2)
		file_name += ".spr";

	return file_name;
}

void clr_ind(vector<vector<vector<int>>> sprite, int frame, vector<int> bright, int x, int y, POINT cursor, int n, int height, int f_sizeY) {
	HDC hdc = GetDC(GetConsoleWindow());
	int sym, clr[3]{};
	for (int c = 0; c < 3; c++)
		clr[c] = bright_fx(get_n(sprite, frame, x, y, n + c), bright[frame]);
	sym = get_n(sprite, frame, x, y, 0);
	
	if (sym != 0) {
		string clr_hex = to_hex_str(clr[0], 2) + to_hex_str(clr[1], 2) + to_hex_str(clr[2], 2);
		wstring clr_w(clr_hex.begin(), clr_hex.end());

		SetTextColor(hdc, 0xFFFFFF);
		SetBkColor(hdc, 0x000000);

		int ind_x = cursor.x + 10;
		int ind_y = cursor.y + 10;
		RECT rect;
		GetWindowRect(GetConsoleWindow(), &rect);

		if (ind_x < 0)
			ind_x = 0;
		if (ind_x > rect.right - rect.left - 65)
			ind_x = rect.right - rect.left - 65;
		if (ind_y < 0)
			ind_y = 0;
		if (ind_y > height * f_sizeY)
			ind_y = height * f_sizeY;

		rect.left = ind_x;
		rect.top = ind_y;
		DrawTextW(hdc, clr_w.c_str(), -1, &rect, DT_NOCLIP);
	}
}

void conf_ind(vector<vector<vector<int>>> sprite, int frame, int x, int y, POINT cursor, int n, int height, int f_sizeY) {
	HDC hdc = GetDC(GetConsoleWindow());
	int conf = get_n(sprite, frame, x, y, n);

	if (conf != 0) {
		SetTextColor(hdc, 0xFFFFFF);
		SetBkColor(hdc, 0x000000);

		int ind_x = cursor.x + 10;
		int ind_y = cursor.y + 10;
		RECT rect;
		GetWindowRect(GetConsoleWindow(), &rect);

		if (ind_x < 0)
			ind_x = 0;
		if (ind_x > rect.right - rect.left - 70)
			ind_x = rect.right - rect.left - 70;
		if (ind_y < 0)
			ind_y = 0;
		if (ind_y > height * f_sizeY)
			ind_y = height * f_sizeY;

		string conf_str = to_string(conf);
		if (n == 9) {
			if (conf == 1)
				conf_str = "collision";
		}
		if (n == 10)
			conf_str += "ch";
		if (n == 11)
			conf_str = "update";
		if (n == 12)
			conf_str += '%';
		wstring conf_w(conf_str.begin(), conf_str.end());
		rect.left = ind_x;
		rect.top = ind_y;
		DrawTextW(hdc, conf_w.c_str(), -1, &rect, DT_NOCLIP);
	}
}

void get_cursor(POINT& cursor, int f_sizeX, int f_sizeY, int& x, int& y, int& xc, int& yc, int& initXC, int& initYC) {
	initXC = xc;
	initYC = yc;
	GetCursorPos(&cursor);
	ScreenToClient(GetConsoleWindow(), &cursor);

	x = cursor.x / f_sizeX + 1;
	y = cursor.y / f_sizeY + 1;

	xc = x, yc = y;
}

void get_clr(int* clr) {
	OpenClipboard(GetConsoleWindow());
	string clr_str;
	if ((char*)GetClipboardData(CF_TEXT) != nullptr) {
		clr_str = (char*)GetClipboardData(CF_TEXT);
		CloseClipboard();

		if (clr_str.length() <= 7) {
			while (clr_str.find('#') != string::npos)
				clr_str.erase(clr_str.begin() + clr_str.find('#'));

			if (clr_str.length() > 6)
				clr_str.erase(clr_str.begin() + 7, clr_str.end());

			if (clr_str.length() == 6) {
				for (int i = 4; i >= 0; i -= 2) {
					clr_str.insert(clr_str.begin() + i, '0');
					clr_str.insert(clr_str.begin() + i + 1, 'x');
				}
				for (int i = 0; i < clr_str.length(); i += 4)
					clr[i / 4] = stoi(clr_str.substr(i, 4), 0, 16);
			}
		}
		while (GetAsyncKeyState(VK_F6) && (char*)GetClipboardData(CF_TEXT) != nullptr)
			FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
	}
	CloseClipboard();
}

void cleanup(vector<vector<vector<int>>>& sprite, int width, int height) {
	vector<vector<vector<int>>> spr_change;
	while (spr_change != sprite) {
		spr_change = sprite;
		vector<int> sprite_null(12);
		for (int fr = 0; fr < sprite.size(); fr++) {	//removing dublicates/off-border
			for (int px = 0; px < sprite[fr].size(); px++) {
				if (sprite[fr][px][1] < 1 || sprite[fr][px][1] > width || sprite[fr][px][2] < 1 || sprite[fr][px][2] > height)
					sprite[fr].erase(sprite[fr].begin() + px);
				for (int i = px + 1; i < sprite[fr].size(); i++) {
					if (sprite[fr][px][1] == sprite[fr][i][1] && sprite[fr][px][2] == sprite[fr][i][2])
						sprite[fr].erase(sprite[fr].begin() + i);
				}
			}
		}

		for (int fr = 0; fr < sprite.size(); fr++) {	//removing nulls
			for (int px = 0; px < sprite[fr].size(); px++) {
				if (sprite[fr][px][0] == 0 && is_null(sprite[fr][px], 9, 11))
					sprite[fr].erase(sprite[fr].begin() + px);
			}
		}

		for (int fr = 0; fr < sprite.size(); fr++) {	//adding null if necessary
			if (sprite[fr].size() == 0)
				sprite[fr].push_back(sprite_null);
		}
	}
}

bool menu(vector<vector<vector<int>>>& sprite, vector<int>& delay, vector<string>& audio, string& sprite_name, int& width, int& height, int& f_sizeY, int& f_sizeX, int arg_c, char** arg_l) {
	int cons[] = { width, height, f_sizeY, f_sizeX };
	int consMin[] = { 1, 1, 6, 4 };
	int consMax[] = { 0, 0, 99, 99 };

	int part = 0;
	int curLim[] = { 4, 8 };

	int curX = 20;
	int curY = 4;

	string imp_path;

	console_config(cons[0], cons[1], cons[2], cons[3], FW_NORMAL, 1, L"Terminal");
	while (true) {
		console_config(cons[0], cons[1], cons[2], cons[3], FW_NORMAL, 0, L"Terminal", L"sprite_designer_8");
		char in{};
		if (arg_c < 2) {
			printf("\033[0m\033[?25l");
			string name_tmp = sprite_name;
			string imp_tmp = imp_path;

			if (sprite_name.length() > 25)
				name_tmp = ".." + string(sprite_name.end() - 25, sprite_name.end());
			if (imp_path.length() > 17)
				imp_tmp = ".." + string(imp_path.end() - 17, imp_path.end());

			if (part == 0)
				printf("\033[4;5HNew Sprite\033[6;5HOpen Sprite\033[8;5HExit\033[19;39Hby areg-77");
			if (part == 1) {
				RECT rect;
				GetWindowRect(GetConsoleWindow(), &rect);

				printf("\033[4;5HSprite: %s\033[6;5HWidth: %dpx \033[38;5;240m(%d)\033[0m \033[8;5HHeight: %dpx \033[38;5;240m(%d)\033[0m \033[10;5HFont sizeY: %d \033[12;5HFont sizeX: %d \033[14;5HImport configs: %s\033[16;5HCreate", name_tmp.c_str(), cons[0], rect.right - rect.left, cons[1], rect.bottom - rect.top - GetSystemMetrics(SM_CYCAPTION), cons[2], cons[3], imp_tmp.c_str());

				printf("\033[%d;%dH\333", cons[1], cons[0]);
			}
			if (part == 2)
				printf("\033[4;5HSprite: %s\033[6;5HOpen", name_tmp.c_str());

			printf("\033[%d;%dH<", curY, curX);

			in = _getch();
			printf("\033[%d;%dH ", curY, curX);

			if ((in == 'w' || in == 72) && curY > curLim[0])	//up
				curY -= 2;
			if ((in == 's' || in == 80) && curY < curLim[1])	//down
				curY += 2;

			if ((in == 'd' || in == 77) && curY != 4 && part > 0) {	//increase
				if (curY == 6)
					cons[0]++;
				if (curY == 8)
					cons[1]++;
				if (curY == 10 && cons[2] < consMax[2])
					cons[2]++;
				if (curY == 12 && cons[3] < consMax[3])
					cons[3]++;
				system("cls");
				console_config(cons[0], cons[1], cons[2], cons[3], FW_NORMAL, 1, L"Terminal");
			}
			if ((in == 'a' || in == 75) && curY != 4 && part > 0) {	//decrease
				if (curY == 6 && cons[0] > consMin[0])
					cons[0]--;
				if (curY == 8 && cons[1] > consMin[1])
					cons[1]--;
				if (curY == 10 && cons[2] > consMin[2])
					cons[2]--;
				if (curY == 12 && cons[3] > consMin[3])
					cons[3]--;
				system("cls");
				console_config(cons[0], cons[1], cons[2], cons[3], FW_NORMAL, 1, L"Terminal");
			}

			if ((in == 'r' || in == 63) && part == 1) {	//refresh
				system("cls");
				console_config(cons[0], cons[1], cons[2], cons[3], FW_NORMAL, 1, L"Terminal");
			}
		}

		if (in == VK_RETURN || arg_c >= 2) {	//press
			if (part == 1 || part == 2 || arg_c >= 2) {	//cofigurations
				if (curY == 4) {	//sprite
					if (part == 1)
						sprite_name = get_file(1);
					if (part == 2)
						sprite_name = get_file(0);
					replace(sprite_name.begin(), sprite_name.end(), '\\', '/');
					if (sprite_name == "" && part == 1)
						sprite_name = "untitled.spr";
					system("cls");
				}
				if (curY == 14 && part == 1) {	//import configs
					imp_path = get_file(0);

					ifstream file;
					file.open(imp_path);
					if (file.is_open()) {
						for (int i = 0; i < 4; i++)
							file >> cons[i];
					}
					else {
						imp_path.clear();
						cons[0] = width;
						cons[1] = height;
						cons[2] = f_sizeY;
						cons[3] = f_sizeX;
					}
					system("cls");
					console_config(cons[0], cons[1], cons[2], cons[3], FW_NORMAL, 1, L"Terminal");
				}
				if (curY == 16 && part == 1) {	//create
					width = cons[0];
					height = cons[1];
					f_sizeY = cons[2];
					f_sizeX = cons[3];

					ifstream file;
					file.open(sprite_name);
					if (file.is_open()) {
						file.close();
						int ans = MessageBoxA(GetConsoleWindow(), "File with that name already exists.\n\nDo you want to replace it?", "sprite_designer_8", MB_ICONWARNING | MB_OKCANCEL | MB_DEFBUTTON2);
						if (ans == IDOK)
							return 0;
					}
					else
						return 0;
				}
				if ((curY == 6 && sprite_name != "" && part == 2) || arg_c >= 2) {	//opem
					if (arg_c >= 2)
						sprite_name = arg_l[1];
					ifstream file;
					file.open(sprite_name);
					if (file.is_open()) {
						int frames, pixels, del;
						string aud;
						string sym_hex;
						int symbol, x, y, collis, trigg, upd;
						string clrf_hex, clrb_hex;
						int clr_f[3]{}, clr_b[3]{};

						file >> width >> height >> f_sizeY >> f_sizeX;

						file >> frames;
						for (int fr = 0; fr < frames; fr++) {	//reading sprite
							file >> pixels >> del;
							getline(file, aud);
							aud.erase(0, 1);
							delay.push_back(del);
							audio.push_back(aud);
							vector<vector<int>> sprite_px;
							for (int px = 0; px < pixels; px++) {
								file >> sym_hex >> x >> y >> clrf_hex >> clrb_hex >> collis >> trigg >> upd;
								symbol = stoi(sym_hex, 0, 16);

								while (clrf_hex.length() < 6)
									clrf_hex = '0' + clrf_hex;
								while (clrb_hex.length() < 6)
									clrb_hex = '0' + clrb_hex;
								for (int i = 0; i < clrf_hex.length(); i += 2) {
									clr_f[i / 2] = stoi(clrf_hex.substr(i, 2), 0, 16);
									clr_b[i / 2] = stoi(clrb_hex.substr(i, 2), 0, 16);
								}

								vector<int> sprite_s = { symbol, x, y, clr_f[0], clr_f[1], clr_f[2], clr_b[0], clr_b[1], clr_b[2] , collis, trigg, upd };

								sprite_px.push_back(sprite_s);
							}
							sprite.push_back(sprite_px);
						}

						file.close();

						return 1;
					}
					else {
						arg_c = 0;
						MessageBoxA(GetConsoleWindow(), "Error occured while opening the specified file", sprite_name.c_str(), MB_ICONERROR);
					}
				}
			}
			if (part == 0) {	//main menu
				system("cls");
				if (curY == 4) {	//new sprite
					part = 1;
					curLim[0] = 4;
					curLim[1] = 16;
					curX = 42;
					curY = 4;

					sprite_name = "untitled.spr";
				}
				if (curY == 6) {	//open sprite
					part = 2;
					curLim[0] = 4;
					curLim[1] = 6;
					curX = 42;
					curY = 4;

					sprite_name = "";
				}
				if (curY == 8)	//exit
					exit(0);
			}
		}
		if ((in == VK_ESCAPE || in == 8) && part > 0) {	//back
			part = 0;
			curLim[0] = 4;
			curLim[1] = 8;
			curX = 20;
			curY = 4;

			imp_path.clear();
			cons[0] = width;
			cons[1] = height;
			cons[2] = f_sizeY;
			cons[3] = f_sizeX;

			sprite_name = "untitled.spr";
			system("cls");
			console_config(cons[0], cons[1], cons[2], cons[3], FW_NORMAL, 1, L"Terminal");
		}
	}
}

void preview(vector<vector<vector<int>>> sprite, vector<int> delay, vector<string> audio, vector<int> bright) {
	FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
	int frame = 0;
	bool inb_cls = true;
	system("cls");
	printf("\033[?25l");
	while (GetAsyncKeyState(VK_SPACE) || GetAsyncKeyState(VK_MENU) || GetAsyncKeyState(VK_CONTROL))
		FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
	while (true) {
		if (inb_cls)
			SetConsoleTitle((L"preview [frame: " + to_wstring(frame + 1) + L']').c_str());
		else
			SetConsoleTitle((L"preview [frame: " + to_wstring(frame + 1) + L"] (no CLS)").c_str());
		for (int i = 0; i < sprite[frame].size(); i++) {	//draw
			if (sprite[frame][i][0] != 0)
				printf("\033[38;2;%d;%d;%dm\033[48;2;%d;%d;%dm\033[%d;%dH%c\033[0m", bright_fx(sprite[frame][i][3], bright[frame]), bright_fx(sprite[frame][i][4], bright[frame]), bright_fx(sprite[frame][i][5], bright[frame]), bright_fx(sprite[frame][i][6], bright[frame]), bright_fx(sprite[frame][i][7], bright[frame]), bright_fx(sprite[frame][i][8], bright[frame]), sprite[frame][i][2], sprite[frame][i][1], char(sprite[frame][i][0]));
		}
		if (audio[frame] != "0") {
			wstring waudio(audio[frame].begin(), audio[frame].end());
			PlaySoundW(waudio.c_str(), NULL, SND_ASYNC);
		}
		sleep_for(chrono::milliseconds(delay[frame]));

		if (GetAsyncKeyState(VK_MENU) && GetConsoleWindow() == GetForegroundWindow()) {	//in-betwwen cls
			if (inb_cls)
				inb_cls = false;
			else
				inb_cls = true;
			while (GetAsyncKeyState(VK_MENU))
				FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
		}
		if (GetAsyncKeyState(VK_CONTROL) && GetConsoleWindow() == GetForegroundWindow()) {	//pause
			while (GetAsyncKeyState(VK_CONTROL))
				FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
		}
		if (inb_cls)
			system("cls");

		frame++;
		if (frame >= sprite.size())
			frame = 0;

		if ((GetAsyncKeyState(VK_SPACE) || GetAsyncKeyState(VK_ESCAPE)) && GetConsoleWindow() == GetForegroundWindow())
			break;
	}
	system("cls");
	PlaySoundW(0, NULL, SND_ASYNC);

	while (GetAsyncKeyState('Z') || GetAsyncKeyState(VK_MULTIPLY) || GetAsyncKeyState(VK_DIVIDE) || GetAsyncKeyState('V') || GetAsyncKeyState('C') || GetAsyncKeyState(VK_TAB) || GetAsyncKeyState(VK_SPACE) || GetAsyncKeyState(VK_INSERT) || GetAsyncKeyState(VK_END) || GetAsyncKeyState(VK_ESCAPE) || GetAsyncKeyState(VK_RETURN) || GetAsyncKeyState(VK_BACK) || GetAsyncKeyState(VK_CONTROL) || GetAsyncKeyState(VK_MENU) || GetAsyncKeyState(VK_DELETE) || GetAsyncKeyState(VK_LSHIFT) || GetAsyncKeyState(VK_RSHIFT) || GetAsyncKeyState(VK_F10) || GetAsyncKeyState(VK_F12))	//flushing tab/space/esc/enter/.. buffer
		FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
}

bool has_changes(vector<vector<vector<int>>> sprite, vector<int> delay, vector<string> audio, vector<int> bright, int width, int height, int f_sizeY, int f_sizeX, string sprite_name) {
	vector<vector<vector<int>>> tmp_sprite;
	vector<int> tmp_delay;
	vector<string> tmp_audio;

	int tmp_width, tmp_height, tmp_f_sizeY, tmp_f_sizeX, frames, pixels, del;
	string aud;
	string tmp_sym_hex;
	int symbol, x, y, collis, trigg, upd;
	string tmp_clrf_hex, tmp_clrb_hex;
	int clr_f[3]{}, clr_b[3]{};

	bool change = false;

	ifstream tmp_file;
	tmp_file.open(sprite_name);
	if (tmp_file.is_open()) {
		tmp_file >> tmp_width >> tmp_height >> tmp_f_sizeY >> tmp_f_sizeX;

		tmp_file >> frames;
		if (frames != sprite.size())
			change = true;
		if (!change) {
			for (int fr = 0; fr < frames; fr++) {	//reading sprite
				tmp_file >> pixels >> del;
				getline(tmp_file, aud);
				aud.erase(0, 1);
				tmp_delay.push_back(del);
				tmp_audio.push_back(aud);
				vector<vector<int>> sprite_px;
				for (int px = 0; px < pixels; px++) {
					tmp_file >> tmp_sym_hex >> x >> y >> tmp_clrf_hex >> tmp_clrb_hex >> collis >> trigg >> upd;
					symbol = stoi(tmp_sym_hex, 0, 16);

					while (tmp_clrf_hex.length() < 6)
						tmp_clrf_hex = '0' + tmp_clrf_hex;
					while (tmp_clrb_hex.length() < 6)
						tmp_clrb_hex = '0' + tmp_clrb_hex;
					for (int i = 0; i < tmp_clrf_hex.length(); i += 2) {
						clr_f[i / 2] = bright_fx(stoi(tmp_clrf_hex.substr(i, 2), 0, 16), bright[fr]);
						clr_b[i / 2] = bright_fx(stoi(tmp_clrb_hex.substr(i, 2), 0, 16), bright[fr]);
					}

					vector<int> sprite_s = { symbol, x, y, clr_f[0], clr_f[1], clr_f[2], clr_b[0], clr_b[1], clr_b[2] , collis, trigg, upd };

					sprite_px.push_back(sprite_s);
				}
				tmp_sprite.push_back(sprite_px);
			}
		}
		tmp_file.close();

		if (!change && (tmp_width != width || tmp_height != height || tmp_f_sizeY != f_sizeY || tmp_f_sizeX != f_sizeX || tmp_sprite != sprite || tmp_delay != delay || tmp_audio != audio))
			change = true;
	}
	else
		change = false;
	return change;
}

int sprite_menu(vector<vector<vector<int>>>& sprite, vector<int>& delay, vector<string>& audio, vector<int>& bright, string& sprite_name, wstring& sprite_title, int& width, int& height, int& f_sizeY, int& f_sizeX, int frame, bool quick_save = false) {
	if (!quick_save) {
		while (GetAsyncKeyState(VK_ESCAPE))
			FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
		system("cls");
		FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
		console_config(50, 20, 24, 10, FW_NORMAL, 1, L"Terminal", L"sprite_designer_8");
		system("cls");
		printf("\033[0m\033[?25l");
	}

	int part = 0;
	int curX = 24;
	int curY = 4;

	int minY = 4, maxY = 10;

	vector<vector<vector<int>>> imp_sprite;
	string imp_sprite_name;
	int imp_frMin = 0, imp_frMax = 0;
	int min_fr = 0, max_fr = 0;
	string arg[] = { "ADD", "CUT" };
	int ar = 0;
	int incdec = 0;

	while (true) {
		char in{};
		if (!quick_save) {
			if (part == 0)
				printf("\033[4;5HSave Sprite\033[6;5HSave Sprite As\033[8;5HImport Sprite\033[10;5HExit");
			if (part == 1) {
				string und[2];
				und[incdec] = "\033[4m";
				string tmp_imp_name = imp_sprite_name;
				if (imp_sprite_name.length() > 25)
					tmp_imp_name = ".." + string(imp_sprite_name.end() - 25, imp_sprite_name.end());
				printf("\033[4;5HSprite: %s\033[6;5HFrames: %s%d\033[24m-%s%d\033[24m\033[8;5HIf frames exceeded: %s\033[10;5HImport", tmp_imp_name.c_str(), und[0].c_str(), imp_frMin, und[1].c_str(), imp_frMax, arg[ar].c_str());
			}
			printf("\033[%d;%dH<", curY, curX);

			in = _getch();
			printf("\033[%d;%dH ", curY, curX);

			if ((in == 'w' || in == 72) && curY > minY)	//up
				curY -= 2;
			if ((in == 's' || in == 80) && curY < maxY)	//down
				curY += 2;

			if ((in == 'd' || in == 77)) {	//increase
				if (incdec == 1) {
					if (imp_frMax < max_fr)
						imp_frMax++;
				}
				else {
					if (imp_frMin < max_fr && imp_frMin < imp_frMax)
						imp_frMin++;
				}
			}
			if ((in == 'a' || in == 75)) {	//decrease
				if (incdec == 1) {
					if (imp_frMax > min_fr && imp_frMax > imp_frMin)
						imp_frMax--;
				}
				else {
					if (imp_frMin > min_fr)
						imp_frMin--;
				}
			}
		}

		if (in == VK_RETURN || quick_save) {	//press
			if (((curY == 4 || curY == 6) && part == 0) || quick_save) {	//save sprite/save sprite as
				string tmp_name = sprite_name;
				int counter = 0;
				for (int fr = 0; fr < bright.size(); fr++) {
					if (bright[fr] != 100)
						counter++;
				}
				if (counter > 0) {
					int ans = MessageBoxA(GetConsoleWindow(), "Some frames will be resterized.\n\nSave anyway?", "sprite_designer_8", MB_ICONWARNING | MB_OKCANCEL | MB_DEFBUTTON2);
					if (ans == IDCANCEL) {
						return 0;
						tmp_name = "";
					}
				}
				if (curY == 6) {
					tmp_name = get_file(1);
					if (tmp_name != "") {
						ifstream file_test;
						file_test.open(tmp_name);
						if (file_test.is_open()) {
							file_test.close();
							int ans = MessageBoxA(GetConsoleWindow(), "File with that name already exists.\n\nDo you want to replace it?", "sprite_designer_8", MB_ICONWARNING | MB_OKCANCEL | MB_DEFBUTTON2);
							if (ans == IDCANCEL) {
								tmp_name = "";
								return 0;
							}
						}

						sprite_name = tmp_name;
						sprite_title = wstring(sprite_name.begin(), sprite_name.end());
						if (sprite_name.find(92) != string::npos || sprite_name.find(47) != string::npos) {	//getting title
							for (int i = sprite_name.size() - 1; i >= 0; i--) {
								if (sprite_name[i] == 92 || sprite_name[i] == 47) {
									sprite_title = wstring(sprite_name.begin() + i + 1, sprite_name.end());
									break;
								}
							}
						}
					}
				}
				if (tmp_name != "") {
					ofstream file;
					file.open(tmp_name);
					if (file.is_open()) {
						file << width << ' ' << height << ' ' << f_sizeY << ' ' << f_sizeX << ' ' << sprite.size();

						for (int fr = 0; fr < sprite.size(); fr++) {
							file << ' ' << sprite[fr].size() << ' ' << delay[fr] << ' ' << audio[fr] << '\n';
							for (int px = 0; px < sprite[fr].size(); px++) {
								string sym_hex = to_hex_num(sprite[fr][px][0]);
								if (px != 0)
									file << ' ';
								file << sym_hex << ' ' << sprite[fr][px][1] << ' ' << sprite[fr][px][2] << ' ';
								string clrf_hex, clrb_hex;
								for (int i = 3; i <= 5; i++) {
									clrf_hex += to_hex_str(bright_fx(sprite[fr][px][i], bright[fr]), 2);
									clrb_hex += to_hex_str(bright_fx(sprite[fr][px][i + 3], bright[fr]), 2);
								}
								while (clrf_hex[0] == '0' && clrf_hex.length() > 1)
									clrf_hex.erase(clrf_hex.begin());
								while (clrb_hex[0] == '0' && clrb_hex.length() > 1)
									clrb_hex.erase(clrb_hex.begin());
								file << clrf_hex << ' ' << clrb_hex << ' ' << sprite[fr][px][9] << ' ' << sprite[fr][px][10] << ' ' << sprite[fr][px][11];
							}
						}
						file.close();
						return 0;
					}
					else {
						MessageBoxA(GetConsoleWindow(), "Error occurred while saving the file", "sprite_designer_8", MB_ICONERROR);
						return 0;
					}
				}
			}
			if (curY == 4 && part == 1) {	//sprite (import)
				imp_sprite_name = get_file(0);

				ifstream file;
				file.open(imp_sprite_name);
				if (file.is_open()) {
					imp_sprite.clear();
					int tmp_w, tmp_h, tmp_fY, tmp_fX;
					int frames, pixels, del;
					string aud;
					string sym_hex;
					int symbol, x, y, collis, trigg, upd;
					string clrf_hex, clrb_hex;
					int clr_f[3]{}, clr_b[3]{};

					file >> tmp_w >> tmp_h >> tmp_fY >> tmp_fX;

					file >> frames;
					imp_frMin = 1, imp_frMax = frames;
					min_fr = 1, max_fr = frames;

					for (int fr = 0; fr < frames; fr++) {	//reading sprite
						file >> pixels >> del;
						getline(file, aud);
						vector<vector<int>> sprite_px;
						for (int px = 0; px < pixels; px++) {
							file >> sym_hex >> x >> y >> clrf_hex >> clrb_hex >> collis >> trigg >> upd;
							symbol = stoi(sym_hex, 0, 16);

							while (clrf_hex.length() < 6)
								clrf_hex = '0' + clrf_hex;
							while (clrb_hex.length() < 6)
								clrb_hex = '0' + clrb_hex;
							for (int i = 0; i < clrf_hex.length(); i += 2) {
								clr_f[i / 2] = stoi(clrf_hex.substr(i, 2), 0, 16);
								clr_b[i / 2] = stoi(clrb_hex.substr(i, 2), 0, 16);
							}

							vector<int> sprite_s = { symbol, x, y, clr_f[0], clr_f[1], clr_f[2], clr_b[0], clr_b[1], clr_b[2] , collis, trigg, upd };

							sprite_px.push_back(sprite_s);
						}
						imp_sprite.push_back(sprite_px);
					}

					file.close();
				}
				else {
					imp_frMin = 0, imp_frMax = 0;
					min_fr = 0, max_fr = 0;
					imp_sprite.clear();
					if (imp_sprite_name != "")
						MessageBoxA(GetConsoleWindow(), "Error occured while opening the specified file", sprite_name.c_str(), MB_ICONERROR);
				}
				system("cls");
			}
			if (curY == 6 && part == 1) {	//incdec switch
				incdec++;
				if (incdec > 1)
					incdec = 0;
			}
			if (curY == 8) {
				if (part == 1) {	//if frames exceeded
					ar++;
					if (ar > 1)
						ar = 0;
				}
				if (part == 0) {	//import sprite
					system("cls");

					curX = 42;
					curY = 4;

					part = 1;
				}
			}
			if (curY == 10) {
				if (part == 1 && imp_sprite_name != "") {	//import
					for (int er = imp_sprite.size(); er > imp_frMax; er--)
						imp_sprite.pop_back();
					for (int er = 0; er < imp_frMin - 1; er++)
						imp_sprite.erase(imp_sprite.begin());

					if (imp_sprite.size() + frame > sprite.size()) {
						if (ar == 0) {
							while (imp_sprite.size() + frame > sprite.size())
								add_frame(sprite, delay, audio, bright);
						}
						if (ar == 1) {
							while (imp_sprite.size() + frame > sprite.size())
								imp_sprite.pop_back();
						}
					}

					for (int fr = 0; fr < imp_sprite.size(); fr++) {
						for (int i = 0; i < imp_sprite[fr].size(); i++) {
							if (imp_sprite[fr][i][1] >= 1 && imp_sprite[fr][i][1] <= width && imp_sprite[fr][i][2] >= 1 && imp_sprite[fr][i][2] <= height) {
								int counter = 0;
								for (int px = 0; px < sprite[frame + fr].size(); px++) {
									if (sprite[frame + fr][px][1] == imp_sprite[fr][i][1] && sprite[frame + fr][px][2] == imp_sprite[fr][i][2] && (!is_null(sprite[frame + fr][px], 9, 11) || sprite[frame + fr][px][0] == 0 || imp_sprite[fr][i][0] != 0 || !is_null(imp_sprite[fr][i], 9, 11))) {
										if (!is_null(imp_sprite[fr][i], 9, 11)) {
											counter++;
											for (int l = 9; l < 12; l++)
												sprite[frame + fr][px][l] = imp_sprite[fr][i][l];
										}
										if (!is_null(imp_sprite[fr][i], 3, 8)) {
											counter++;
											sprite[frame + fr][px][0] = imp_sprite[fr][i][0];
											for (int c = 3; c <= 8; c++)
												sprite[frame + fr][px][c] = imp_sprite[fr][i][c];
										}
									}
								}
								if (counter == 0)
									sprite[frame + fr].push_back(imp_sprite[fr][i]);
							}
						}
					}
					return 1;
				}
				if (part == 0) {	//exit
					if (has_changes(sprite, delay, audio, bright, width, height, f_sizeY, f_sizeX, sprite_name)) {
						int ans = MessageBoxA(GetConsoleWindow(), "No changes were saved.\n\nExit anyway?", "sprite_designer_8", MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2);
						if (ans == IDNO)
							return 0;
					}
					exit(0);
				}
			}
		}
		if (in == VK_ESCAPE || in == VK_BACK) {	//back
			if (part == 0)
				return 0;
			if (part == 1) {
				system("cls");
				part = 0;
				curX = 24;
				curY = 4;

				minY = 4, maxY = 10;
			}
		}
	}
}

int main(int arg_c, char** arg_l) {
	setlocale(LC_ALL, "C");
	srand(time(0));
	system("cls");
	vector<vector<vector<int>>> sprite;
	vector<int> delay;
	vector<int> bright;
	vector<string> audio;

	string sprite_name;
	int width = 50, height = 20, f_sizeY = 24, f_sizeX = 10;
	bool opened = menu(sprite, delay, audio, sprite_name, width, height, f_sizeY, f_sizeX, arg_c, arg_l);
	while (GetAsyncKeyState(VK_MULTIPLY) || GetAsyncKeyState(VK_DIVIDE) || GetAsyncKeyState('V') || GetAsyncKeyState('C') || GetAsyncKeyState(VK_TAB) || GetAsyncKeyState(VK_END) || GetAsyncKeyState(VK_INSERT) || GetAsyncKeyState(VK_SPACE) || GetAsyncKeyState(VK_ESCAPE) || GetAsyncKeyState(VK_RETURN) || GetAsyncKeyState(VK_BACK) || GetAsyncKeyState(VK_LSHIFT) || GetAsyncKeyState(VK_RSHIFT) || GetAsyncKeyState(VK_F10) || GetAsyncKeyState(VK_F12) || GetAsyncKeyState(VK_F6) || GetAsyncKeyState('W') || GetAsyncKeyState('A') || GetAsyncKeyState('S') || GetAsyncKeyState('D'))	//flushing tab/space/esc/enter/.. buffer
		FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));

	wstring sprite_title = wstring(sprite_name.begin(), sprite_name.end());
	if (sprite_name.find(92) != string::npos || sprite_name.find(47) != string::npos) {	//getting title
		for (int i = sprite_name.size() - 1; i >= 0; i--) {
			if (sprite_name[i] == 92 || sprite_name[i] == 47) {
				sprite_title = wstring(sprite_name.begin() + i + 1, sprite_name.end());
				break;
			}
		}
	}

	int frame = 0;
	if (!opened)
		add_frame(sprite, delay, audio, bright);
	else {
		for (int i = 0; i < sprite.size(); i++)
			bright.push_back(100);
	}
	system("cls");

	int sym = 219;
	int conf[] = { 1, 1 };
	int x = 1, y = 1;
	int clr_f[] = { 255, 255, 255 };
	int clr_b[] = { 0, 0, 0 };
	bool config_mode = false;
	bool prevframe_mode = false;
	int prevframe_op = 50;

	bool audio_path = false;
	bool lock_frames = false;
	bool hex_mode = true;
	int size_format = 1;
	if (opened)
		lock_frames = true;

	bool update_screen = true;
	int xtr_height = height + 7;

	int incdec = 0;

	vector<vector<int>> copy_sprite;

	vector<vector<vector<vector<int>>>> undo_sprite;
	int undo_pos = 0;

	vector<vector<vector<int>>> color_sprite;

	chrono::milliseconds speed(1);

	console_config(width, xtr_height, f_sizeY, f_sizeX, FW_NORMAL, 1, L"Terminal");
	while (true) {
		FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
		while (GetAsyncKeyState('S'))
			FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
		xtr_height = height + 7;
		while (audio_path && (((!lock_frames && audio[frame].length() + 4) / width > xtr_height - height - 5) || (lock_frames && (audio[frame].length() + 6) / width > xtr_height - height - 5)))
			xtr_height++;

		cleanup(sprite, width, height);
		for (int fr = 0; fr < sprite.size(); fr++)
			sort(sprite[fr].begin(), sprite[fr].end());
		for (int fr = 0; fr < audio.size(); fr++) {
			if (audio[fr].empty())
				audio[fr] = "0";
		}

		if (config_mode)
			console_config(width, xtr_height, f_sizeY, f_sizeX, FW_NORMAL, 0, L"Terminal", (L"configuration mode (" + to_wstring(width) + L'x' + to_wstring(height) + L"px) (" + to_wstring(f_sizeY) + L':' + to_wstring(f_sizeX) + L"f)").c_str());
		else
			console_config(width, xtr_height, f_sizeY, f_sizeX, FW_NORMAL, 0, L"Terminal", (sprite_title + L" (" + to_wstring(width) + L'x' + to_wstring(height) + L"px) (" + to_wstring(f_sizeY) + L':' + to_wstring(f_sizeX) + L"f)").c_str());

		UI(frame, sym, conf, x, y, clr_f, clr_b, config_mode, lock_frames, audio_path, delay, audio, bright, sprite, incdec, hex_mode, size_format, width, height, f_sizeY, f_sizeX);

		printf("\033[?25h\033[?12l\033[%d;%dH", y, x);
		sleep_for(speed);

		vector<vector<vector<int>>> sprite_change = sprite;

		if (GetConsoleWindow() == GetForegroundWindow()) {
			if (!(GetAsyncKeyState(VK_LBUTTON) || GetAsyncKeyState(VK_MBUTTON))) {	//undo sprite history
				if (undo_sprite.size() == 0) {
					undo_sprite.push_back(sprite);
					undo_pos = undo_sprite.size() - 1;
				}
				if (undo_sprite[undo_sprite.size() - 1] != sprite) {
					undo_sprite.push_back(sprite);
					undo_pos = undo_sprite.size() - 1;
				}
			}
			if (undo_sprite.size() > 100) {
				undo_sprite.erase(undo_sprite.begin());
				undo_pos = undo_sprite.size() - 1;
			}

			int initX = x, initY = y;
			int xc, yc, initXC, initYC;

			POINT cursor;
			get_cursor(cursor, f_sizeX, f_sizeY, x, y, xc, yc, initXC, initYC);

			if (GetAsyncKeyState(VK_MBUTTON) && GetAsyncKeyState(VK_LMENU)) {
				update_screen = true;
				if (yc - initY < 0 && min_elem(sprite, frame, 2) >= 1 - (yc - initY))	//global up
					global_pos(sprite, frame, 0, yc - initY);
				if (xc - initX < 0 && min_elem(sprite, frame, 1) >= 1 - (xc - initX))	//global left
					global_pos(sprite, frame, xc - initX, 0);
				if (yc - initY > 0 && max_elem(sprite, frame, 2) <= height - (yc - initY))	//global down
					global_pos(sprite, frame, 0, yc - initY);
				if (xc - initX > 0 && max_elem(sprite, frame, 1) <= width - (xc - initX))	//global right
					global_pos(sprite, frame, xc - initX, 0);

				vector<int> sprite_null(12);
				if (count(sprite[frame].begin(), sprite[frame].end(), sprite_null) == 0) {
					x = min_elem(sprite, frame, 1) + (max_elem(sprite, frame, 1) - min_elem(sprite, frame, 1)) / 2;
					y = min_elem(sprite, frame, 2) + (max_elem(sprite, frame, 2) - min_elem(sprite, frame, 2)) / 2;
				}
			}

			if (x < 1)	//border check
				x = 1;
			if (x > width)
				x = width;
			if (y < 1)
				y = 1;
			if (y > height)
				y = height;

			if (GetAsyncKeyState(VK_LMENU) && !GetAsyncKeyState(VK_MBUTTON)) {	//color indicator
				if (config_mode)
					conf_ind(sprite, frame, x, y, cursor, 8 + conf[0], height, f_sizeY);
				else {
					if (GetAsyncKeyState(VK_LCONTROL))
						clr_ind(sprite, frame, bright, x, y, cursor, 6, height, f_sizeY);
					else
						clr_ind(sprite, frame, bright, x, y, cursor, 3, height, f_sizeY);
				}
			}

			if (GetAsyncKeyState(VK_LBUTTON) && initXC == xc && initYC == yc) {	//clickable modes
				if (xc >= width - 11 && ((hex_mode && xc <= width - 5) || (!hex_mode && xc <= width - 1)) && (yc == height + 3 || yc == height + 4) && !config_mode) {	//clr hex mode
					if (hex_mode)
						hex_mode = false;
					else
						hex_mode = true;
					while (GetAsyncKeyState(VK_LBUTTON))
						FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				}
				if (xc >= width - 11 && xc <= width - 1 && yc == height + 6) {	//size format
					size_format++;
					if (size_format > 3)
						size_format = 0;
					while (GetAsyncKeyState(VK_LBUTTON))
						FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				}
				string bright_tmp = to_string(bright[frame] - 100) + '%';
				if (bright[frame] - 100 > 0)
					bright_tmp = '+' + bright_tmp;
				string resterize_box = ' ' + to_string(delay[frame]) + "ms ";
				if (xc >= 1 + resterize_box.length() && xc <= 1 + resterize_box.length() + bright_tmp.length() - 1 && yc == height + 6 && !config_mode) {	//resterize frame
					if (GetAsyncKeyState(VK_LCONTROL) && sprite.size() > 1) {
						int ans = MessageBoxA(GetConsoleWindow(), "Resterize all frames?", "sprite_designer_8", MB_OKCANCEL);
						while (GetAsyncKeyState(VK_ESCAPE))
							FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
						if (ans == IDOK) {
							for (int fr = 0; fr < sprite.size(); fr++) {
								for (int px = 0; px < sprite[fr].size(); px++) {
									for (int i = 3; i <= 8; i++)
										sprite[fr][px][i] = bright_fx(sprite[fr][px][i], bright[fr]);
								}
								bright[fr] = 100;
							}
						}
					}
					else {
						int ans = MessageBoxA(GetConsoleWindow(), "Resterize current frame?", "sprite_designer_8", MB_OKCANCEL);
						while (GetAsyncKeyState(VK_ESCAPE))
							FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
						if (ans == IDOK) {
							for (int px = 0; px < sprite[frame].size(); px++) {
								for (int i = 3; i <= 8; i++)
									sprite[frame][px][i] = bright_fx(sprite[frame][px][i], bright[frame]);
							}
							bright[frame] = 100;
						}
					}
				}
			}

			if (xc >= 1 && xc <= width && yc >= 1 && yc <= height && cursor.x > 0 && cursor.y > 0) {	//draw
				vector<int> sprite_null(12);
				if (GetAsyncKeyState(VK_RETURN) && GetAsyncKeyState(VK_RCONTROL)) {	//fill
					if (config_mode) {
						int counter = 0;
						for (int i = 0; i < sprite[frame].size(); i++) {	//checking nulls
							if (sprite[frame][i] == sprite_null)
								counter++;
						}

						if (counter == 0) {
							for (int i = 0; i < sprite[frame].size(); i++)
								sprite[frame][i][8 + conf[0]] = conf[1];
						}
					}
					else {
						int c = 1;
						for (int l = 1; l <= height; l++) {
							for (int i = 1; i <= width; i++)
								save_pos(frame, sym, conf, i, l, clr_f, clr_b, config_mode, sprite);
						}
					}
				}
				else if (GetAsyncKeyState(VK_LBUTTON) && !GetAsyncKeyState(VK_LCONTROL))
					save_pos(frame, sym, conf, x, y, clr_f, clr_b, config_mode, sprite);
			}
			if (xc >= 1 && xc <= width && yc >= 1 && yc <= height) {	//erase
				printf("\033[?25l");
				if (GetAsyncKeyState(VK_BACK) && GetAsyncKeyState(VK_RCONTROL)) {
					for (int l = 1; l <= height; l++) {
						for (int i = 1; i <= width; i++)
							erase_pos(frame, i, l, conf, config_mode, sprite);
					}
					system("cls");
					update_screen = true;
				}
				else if (GetAsyncKeyState(VK_LBUTTON) && GetAsyncKeyState(VK_LCONTROL)) {
					erase_pos(frame, x, y, conf, config_mode, sprite);
					printf("\033[%d;%dH ", y, x);
					update_screen = true;
				}
			}
			if (GetAsyncKeyState(VK_LCONTROL) && GetAsyncKeyState(VK_SPACE)) {	//add audio
				string prepath = get_file(2);
				if (prepath != "")
					audio[frame] = prepath;
				if (audio[frame] == "")
					audio[frame] = "0";
				while (GetAsyncKeyState(VK_ESCAPE) || GetAsyncKeyState(VK_RETURN) || GetAsyncKeyState(VK_BACK) || GetAsyncKeyState(VK_LBUTTON))
					FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
			}
			if (GetAsyncKeyState(VK_RCONTROL) && GetAsyncKeyState(VK_SPACE) && audio_path) {	//clean audio path
				int idx = 0;
				while (audio[frame][idx] != '/')
					idx++;
				if (idx != 0 && idx != audio[frame].length())
					audio[frame].erase(0, idx + 1);
				idx = 0;
				while (audio[frame][idx] == ' ')
					idx++;
				audio[frame].erase(0, idx);
				if (audio[frame] == "")
					audio[frame] = "0";
				while (GetAsyncKeyState(VK_RCONTROL) && GetAsyncKeyState(VK_SPACE))
					FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
			}
			if (GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_INSERT)) {	//hide/show full audio path
				if (audio[frame] != "0") {
					if (audio_path)
						audio_path = false;
					else
						audio_path = true;
				}
				while (GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_INSERT))
					FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
			}

			if (!GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_TAB) && !((conf[0] == 1 || conf[0] == 3) && config_mode)) {	//inc/dec switch
				incdec++;
				if (incdec > 1)
					incdec = 0;
				while (GetAsyncKeyState(VK_TAB))
					FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
			}

			if (GetAsyncKeyState('E') && GetAsyncKeyState('Q') && !config_mode && incdec == 0) {	//reset symbol
				sym = 219;
				while (GetAsyncKeyState('E') || GetAsyncKeyState('Q'))
					FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
			}
			if (GetAsyncKeyState('E') && incdec == 0) {	//increase symbol
				if (config_mode) {
					conf[0]++;
					if (conf[0] > 3)
						conf[0] = 1;
					conf[1] = 1;
				}
				else {
					sym++;
					while ((sym >= 7 && sym <= 15) || sym == 27 || sym == 31 || sym == 32 || sym == 127 || sym == 249)
						sym++;
					if (sym > 254)
						sym = 1;
				}
				if (!GetAsyncKeyState(VK_LCONTROL)) {
					while (GetAsyncKeyState('E'))
						FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				}
				if (config_mode) {
					system("cls");
					update_screen = true;
				}
			}
			if (GetAsyncKeyState('Q') && incdec == 0) {	//decrease symbol
				if (config_mode) {
					conf[0]--;
					if (conf[0] < 1)
						conf[0] = 3;
					conf[1] = 1;
				}
				else {
					sym--;
					while ((sym >= 7 && sym <= 15) || sym == 27 || sym == 31 || sym == 32 || sym == 127 || sym == 249)
						sym--;
					if (sym < 1)
						sym = 254;
				}
				if (!GetAsyncKeyState(VK_LCONTROL)) {
					while (GetAsyncKeyState('Q'))
						FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				}
				if (config_mode) {
					system("cls");
					update_screen = true;
				}
			}

			if (GetAsyncKeyState('Q') && GetAsyncKeyState('E') && !config_mode && incdec == 1) {	//reset clr
				if (GetAsyncKeyState(VK_LCONTROL)) {
					for (int c = 0; c < 3; c++)
						clr_b[c] = 0;
				}
				else {
					for (int c = 0; c < 3; c++)
						clr_f[c] = 255;
				}
				while (GetAsyncKeyState('Q') || GetAsyncKeyState('E'))
					FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
			}
			if (GetAsyncKeyState('E') && incdec == 1) {	//increase clr
				if (config_mode) {
					if ((conf[0] == 2 && conf[1] < 255) || (conf[0] == 3 && conf[1] < 3))
						conf[1]++;
					if (!GetAsyncKeyState(VK_LCONTROL) && !(conf[0] == 1 || conf[0] == 3)) {
						while (GetAsyncKeyState('E'))
							FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
					}
				}
				else {
					int i = 0;
					if (GetAsyncKeyState('1'))
						i = 1;
					if (GetAsyncKeyState('2'))
						i = 2;
					if (GetAsyncKeyState('3'))
						i = 3;

					if (GetAsyncKeyState(VK_LCONTROL)) {
						if (i > 0)
							clr_b[i - 1]++;
						else {
							for (int c = 0; c < 3; c++)
								clr_b[c]++;
						}
					}
					else {
						if (i > 0)
							clr_f[i - 1]++;
						else {
							for (int c = 0; c < 3; c++)
								clr_f[c]++;
						}
					}
					for (int c = 0; c < 3; c++) {
						if (clr_f[c] > 255)
							clr_f[c] = 0;
						if (clr_b[c] > 255)
							clr_b[c] = 0;
					}
					if (!GetAsyncKeyState(VK_RCONTROL)) {
						while (GetAsyncKeyState('E'))
							FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
					}
				}
			}
			if (GetAsyncKeyState('Q') && incdec == 1) {	//decrease clr
				if (config_mode) {
					if (((conf[0] == 2 || conf[0] == 3) && conf[1] > 1))
						conf[1]--;
					if (!GetAsyncKeyState(VK_LCONTROL) && !(conf[0] == 1 || conf[0] == 3)) {
						while (GetAsyncKeyState('Q'))
							FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
					}
				}
				else {
					int i = 0;
					if (GetAsyncKeyState('1'))
						i = 1;
					if (GetAsyncKeyState('2'))
						i = 2;
					if (GetAsyncKeyState('3'))
						i = 3;

					if (GetAsyncKeyState(VK_LCONTROL)) {
						if (i > 0)
							clr_b[i - 1]--;
						else {
							for (int c = 0; c < 3; c++)
								clr_b[c]--;
						}
					}
					else {
						if (i > 0)
							clr_f[i - 1]--;
						else {
							for (int c = 0; c < 3; c++)
								clr_f[c]--;
						}
					}
					for (int c = 0; c < 3; c++) {
						if (clr_f[c] < 0)
							clr_f[c] = 255;
						if (clr_b[c] < 0)
							clr_b[c] = 255;
					}
					if (!GetAsyncKeyState(VK_RCONTROL)) {
						while (GetAsyncKeyState('Q'))
							FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
					}
				}
			}
			if (GetAsyncKeyState('1') && GetAsyncKeyState('2') && GetAsyncKeyState('3')) {	//random clr
				if (GetAsyncKeyState(VK_LCONTROL)) {
					for (int c = 0; c < 3; c++)
						clr_b[c] = rand() % 256;
				}
				else {
					for (int c = 0; c < 3; c++)
						clr_f[c] = rand() % 256;
				}
				while (GetAsyncKeyState('1') || GetAsyncKeyState('2') || GetAsyncKeyState('3'))
					FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
			}
			if (GetAsyncKeyState('X') && !config_mode) {	//inverse clr
				int clr_tmp[] = { clr_f[0], clr_f[1], clr_f[2] };
				for (int c = 0; c < 3; c++) {
					clr_f[c] = clr_b[c];
					clr_b[c] = clr_tmp[c];
				}
				while (GetAsyncKeyState('X'))
					FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
			}

			if (GetAsyncKeyState(VK_F5)) {	//refresh
				if (GetAsyncKeyState(VK_LCONTROL))
					console_config(width, xtr_height, f_sizeY, f_sizeX, FW_NORMAL, 1, L"Terminal");
				printf("\033[?25l");
				system("cls");
				update_screen = true;
			}

			if (GetAsyncKeyState(VK_MULTIPLY)) {	//increase screen size
				if (GetAsyncKeyState(VK_LCONTROL))
					height++;
				else
					width++;
				if (!GetAsyncKeyState(VK_RCONTROL)) {
					while (GetAsyncKeyState(VK_MULTIPLY))
						FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				}
				system("cls");
				update_screen = true;
			}
			if (GetAsyncKeyState(VK_DIVIDE)) {	//decrease screen size
				if (GetAsyncKeyState(VK_LCONTROL)) {
					if (height > 1)
						height--;
				}
				else {
					if (width > 1)
						width--;
				}
				if (!GetAsyncKeyState(VK_RCONTROL)) {
					while (GetAsyncKeyState(VK_DIVIDE))
						FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				}
				system("cls");
				update_screen = true;
			}
			if (GetAsyncKeyState(VK_ADD)) {	//increase font
				if (GetAsyncKeyState(VK_LCONTROL)) {
					if (f_sizeX < 99)
						f_sizeX++;
				}
				else {
					if (f_sizeY < 99)
						f_sizeY++;
				}
				if (!GetAsyncKeyState(VK_RCONTROL)) {
					while (GetAsyncKeyState(VK_ADD))
						FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				}
				system("cls");
				update_screen = true;
			}
			if (GetAsyncKeyState(VK_SUBTRACT)) {	//decrease font
				if (GetAsyncKeyState(VK_LCONTROL)) {
					if (f_sizeX > 4)
						f_sizeX--;
				}
				else {
					if (f_sizeY > 6)
						f_sizeY--;
				}
				if (!GetAsyncKeyState(VK_RCONTROL)) {
					while (GetAsyncKeyState(VK_SUBTRACT))
						FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				}
				system("cls");
				update_screen = true;
			}

			if (GetAsyncKeyState(VK_F9) && GetAsyncKeyState(VK_F8) && !config_mode) {	//reset delay/brightness
				if (GetAsyncKeyState(VK_RMENU)) {
					if (prevframe_mode) {
						prevframe_op = 50;
						update_screen = true;
					}
				}
				else {
					if (GetAsyncKeyState(VK_RCONTROL)) {
						if (GetAsyncKeyState(VK_LMENU)) {
							for (int fr = 0; fr < sprite.size(); fr++)
								bright[fr] = 100;
							update_screen = true;
						}
						else {
							for (int fr = 0; fr < sprite.size(); fr++)
								delay[fr] = 0;
						}
					}
					else {
						if (GetAsyncKeyState(VK_LMENU)) {
							bright[frame] = 100;
							update_screen = true;
						}
						else
							delay[frame] = 0;
					}
				}
				while (GetAsyncKeyState(VK_F9) || GetAsyncKeyState(VK_F8))
					FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
			}
			if (GetAsyncKeyState(VK_F9) && !config_mode) {	//increase delay/brightness
				if (GetAsyncKeyState(VK_RMENU)) {
					if (prevframe_op < 100 && prevframe_mode) {
						prevframe_op++;
						update_screen = true;
					}
				}
				else {
					if (GetAsyncKeyState(VK_LMENU)) {
						if (GetAsyncKeyState(VK_RCONTROL)) {
							int counter = 0;
							for (int i = 0; i < bright.size(); i++) {	//checking
								if (bright[i] >= 200)
									counter++;
							}
							if (counter == 0) {
								for (int i = 0; i < bright.size(); i++)
									bright[i]++;
							}
						}
						else {
							if (bright[frame] < 200)
								bright[frame]++;
						}
						update_screen = true;
					}
					else {
						if (GetAsyncKeyState(VK_RCONTROL)) {
							for (int i = 0; i < delay.size(); i++)
								delay[i] += 10;
						}
						else {
							delay[frame] += 10;
						}
					}
				}
				if (!GetAsyncKeyState(VK_MENU) && !GetAsyncKeyState(VK_LCONTROL) || GetAsyncKeyState(VK_MENU) && GetAsyncKeyState(VK_LCONTROL)) {
					while (GetAsyncKeyState(VK_F9))
						FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				}
			}
			if (GetAsyncKeyState(VK_F8) && !config_mode) {	//decrease delay/brightness
				if (GetAsyncKeyState(VK_RMENU)) {
					if (prevframe_op > 0 && prevframe_mode) {
						prevframe_op--;
						update_screen = true;
					}
				}
				else {
					if (GetAsyncKeyState(VK_LMENU)) {
						if (GetAsyncKeyState(VK_RCONTROL)) {
							int counter = 0;
							for (int i = 0; i < bright.size(); i++) {	//checking
								if (bright[i] <= 0)
									counter++;
							}
							if (counter == 0) {
								for (int i = 0; i < bright.size(); i++)
									bright[i]--;
							}
						}
						else {
							if (bright[frame] > 0)
								bright[frame]--;
						}
						update_screen = true;
					}
					else {
						if (GetAsyncKeyState(VK_RCONTROL)) {
							int counter = 0;
							for (int i = 0; i < delay.size(); i++) {	//checking
								if (delay[i] <= 0)
									counter++;
							}
							if (counter == 0) {
								for (int i = 0; i < delay.size(); i++)
									delay[i] -= 10;
							}
						}
						else {
							if (delay[frame] > 0)
								delay[frame] -= 10;
						}
					}
				}
				if (!GetAsyncKeyState(VK_MENU) && !GetAsyncKeyState(VK_LCONTROL) || GetAsyncKeyState(VK_MENU) && GetAsyncKeyState(VK_LCONTROL)) {
					while (GetAsyncKeyState(VK_F8))
						FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				}
			}

			if (GetAsyncKeyState(VK_LCONTROL) && GetAsyncKeyState('Z') && undo_pos > 0) {	//undo
				vector<vector<vector<int>>> tmp_sprite = sprite;
				undo_pos--;
				sprite = undo_sprite[undo_pos];
				if (tmp_sprite.size() == sprite.size()) {
					for (int fr = 0; fr < tmp_sprite.size(); fr++) {
						if (tmp_sprite[fr] != sprite[fr]) {
							frame = fr;
							break;
						}
					}
				}
				else
					frame = sprite.size() - 1;

				while (delay.size() < sprite.size())
					delay.push_back(0);
				while (audio.size() < sprite.size())
					audio.push_back("0");
				while (bright.size() < sprite.size())
					bright.push_back(100);
				while (delay.size() > sprite.size())
					delay.pop_back();
				while (audio.size() > sprite.size())
					audio.pop_back();
				while (bright.size() > sprite.size())
					bright.pop_back();
				if (frame > sprite.size() - 1)
					frame = sprite.size() - 1;
				undo_sprite.pop_back();

				while (GetAsyncKeyState('Z'))
					FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				system("cls");
				update_screen = true;
			}

			if (GetAsyncKeyState('M')) {	//mirror sprite
				if (GetAsyncKeyState(VK_LCONTROL)) {
					for (int px = 0; px < sprite[frame].size(); px++)
						sprite[frame][px][2] = height - sprite[frame][px][2] + 1;
				}
				else {
					for (int px = 0; px < sprite[frame].size(); px++)
						sprite[frame][px][1] = width - sprite[frame][px][1] + 1;
				}

				while (GetAsyncKeyState('M') || GetAsyncKeyState(VK_LCONTROL))
					FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				system("cls");
				update_screen = true;
			}

			if (GetAsyncKeyState(VK_INSERT) && !GetAsyncKeyState(VK_CONTROL)) {	//lock frames
				if (lock_frames)
					lock_frames = false;
				else
					lock_frames = true;
				while (GetAsyncKeyState(VK_INSERT))
					FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				if (frame == 0) {
					system("cls");
					update_screen = true;
				}
			}
			if (GetAsyncKeyState(VK_RSHIFT) && !GetAsyncKeyState(VK_RCONTROL)) {	//increase frame
				if (frame + 1 == sprite.size() && !(lock_frames || config_mode))
					add_frame(sprite, delay, audio, bright, frame);
				if (!lock_frames)
					frame++;
				else {
					frame++;
					if (frame > sprite.size() - 1)
						frame = 0;
				}
				if (frame > sprite.size() - 1)
					frame = sprite.size() - 1;
				if (!GetAsyncKeyState(VK_LCONTROL)) {
					while (GetAsyncKeyState(VK_RSHIFT))
						FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				}
				system("cls");
				update_screen = true;
			}
			if (GetAsyncKeyState(VK_LSHIFT) && !GetAsyncKeyState(VK_RCONTROL)) {	//decrease frame
				if (frame > 0 && !lock_frames)
					frame--;
				if (lock_frames) {
					frame--;
					if (frame < 0)
						frame = int(sprite.size()) - 1;
				}
				if (!GetAsyncKeyState(VK_LCONTROL)) {
					while (GetAsyncKeyState(VK_LSHIFT))
						FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				}
				system("cls");
				update_screen = true;
			}
			if (GetAsyncKeyState(VK_LSHIFT) && GetAsyncKeyState(VK_RCONTROL) && frame != 0) {	//first frame
				frame = 0;
				while (GetAsyncKeyState(VK_LSHIFT) || GetAsyncKeyState(VK_LCONTROL))
					FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				system("cls");
				update_screen = true;
			}
			if (GetAsyncKeyState(VK_RSHIFT) && GetAsyncKeyState(VK_RCONTROL) && frame != sprite.size() - 1) {	//last frame
				frame = sprite.size() - 1;
				while (GetAsyncKeyState(VK_RSHIFT) || GetAsyncKeyState(VK_LCONTROL))
					FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				system("cls");
				update_screen = true;
			}

			if (GetAsyncKeyState(VK_END) && !lock_frames) {	//add frame here
				addhere_frame(sprite, delay, audio, bright, frame);
				frame++;
				while (GetAsyncKeyState(VK_END))
					FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				system("cls");
				update_screen = true;
			}

			if (GetAsyncKeyState(VK_DELETE) && sprite.size() > 1 && !lock_frames) {	//delete frame
				sprite.erase(sprite.begin() + frame);
				delay.erase(delay.begin() + frame);
				audio.erase(audio.begin() + frame);
				bright.erase(bright.begin() + frame);
				if (frame == sprite.size())
					frame--;
				while (GetAsyncKeyState(VK_DELETE))
					FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				system("cls");
				update_screen = true;
			}
			if (GetAsyncKeyState('C') && !config_mode) {	//copy frame
				int counter = 0;
				for (int px = 0; px < sprite[frame].size(); px++) {	//checking nulls
					if (sprite[frame][px][0] != 0)
						counter++;
				}
				if (counter != 0) {
					copy_sprite.clear();
					for (int i = 0; i < sprite[frame].size(); i++)
						copy_sprite.push_back(sprite[frame][i]);
				}
			}
			if (GetAsyncKeyState('V') && !config_mode) {	//paste frame
				if (copy_sprite.size() > 0) {
					while (GetAsyncKeyState(VK_LBUTTON) || GetAsyncKeyState(VK_RETURN) || GetAsyncKeyState(VK_BACK) || GetAsyncKeyState(VK_ESCAPE))
						FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
					system("cls");
					update_screen = true;
					bool paste = true;
					int x_slip = 0, y_slip = 0;

					vector<vector<int>> copy_tmp = copy_sprite;
					vector<int> copy_bright(bright.size());
					for (int fr = 0; fr < copy_bright.size(); fr++)
						copy_bright[fr] = 100;

					while (!(GetAsyncKeyState(VK_LBUTTON) || GetAsyncKeyState(VK_RETURN)) || GetConsoleWindow() != GetForegroundWindow()) {
						if (GetConsoleWindow() == GetForegroundWindow()) {
							string bright_tmp = to_string(copy_bright[frame] - 100) + '%';
							if (copy_bright[frame] - 100 > 0)
								bright_tmp = '+' + bright_tmp;

							console_config(width, height, f_sizeY, f_sizeX, FW_NORMAL, 0, L"Terminal", (L"pasting [frame: " + to_wstring(frame + 1) + L"] " + wstring(bright_tmp.begin(), bright_tmp.end())).c_str());

							initX = x, initY = y;

							POINT cursor;
							get_cursor(cursor, f_sizeX, f_sizeY, x, y, xc, yc, initXC, initYC);

							if (yc - initY < 0)	//global up
								global_pos(copy_tmp, 0, yc - initY, width, height);
							if (xc - initX < 0)	//global left
								global_pos(copy_tmp, xc - initX, 0, width, height);
							if (yc - initY > 0)	//global down
								global_pos(copy_tmp, 0, yc - initY, width, height);
							if (xc - initX > 0)	//global right
								global_pos(copy_tmp, xc - initX, 0, width, height);

							x = min_elem(copy_tmp, 1) + (max_elem(copy_tmp, 1) - min_elem(copy_tmp, 1)) / 2 - x_slip;
							y = min_elem(copy_tmp, 2) + (max_elem(copy_tmp, 2) - min_elem(copy_tmp, 2)) / 2 - y_slip;

							if (GetAsyncKeyState(VK_RCONTROL)) {	//initial position
								for (int i = 0; i < copy_tmp.size(); i++) {	//clear
									if (copy_tmp[i][0] != 0 && copy_tmp[i][1] >= 1 && copy_tmp[i][1] <= width && copy_tmp[i][2] >= 1 && copy_tmp[i][2] <= height)
										printf("\033[%d;%dH ", copy_tmp[i][2], copy_tmp[i][1]);
								}
								copy_tmp = copy_sprite;
							}

							if (GetAsyncKeyState(VK_RSHIFT)) {	//increase frame
								frame++;
								if (frame > sprite.size() - 1) {
									if (lock_frames)
										frame = 0;
									else
										frame = sprite.size() - 1;
								}
								if (!GetAsyncKeyState(VK_LCONTROL)) {
									while (GetAsyncKeyState(VK_RSHIFT))
										FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
								}
								system("cls");
								update_screen = true;
							}
							if (GetAsyncKeyState(VK_LSHIFT)) {	//decrease frame
								frame--;
								if (frame < 0) {
									if (lock_frames)
										frame = frame = int(sprite.size()) - 1;
									else
										frame = 0;
								}
								if (!GetAsyncKeyState(VK_LCONTROL)) {
									while (GetAsyncKeyState(VK_LSHIFT))
										FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
								}
								system("cls");
								update_screen = true;
							}

							if (GetAsyncKeyState(VK_LMENU) && GetAsyncKeyState(VK_F9) && GetAsyncKeyState(VK_F8)) {	//reset brightness
								copy_bright[frame] = 100;
								update_screen = true;
								while (GetAsyncKeyState(VK_F9) || GetAsyncKeyState(VK_F8))
									FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
							}
							if (GetAsyncKeyState(VK_LMENU) && GetAsyncKeyState(VK_F9)) {	//increase brightness
								if (GetAsyncKeyState(VK_RCONTROL)) {
									int counter = 0;
									for (int i = 0; i < copy_bright.size(); i++) {	//checking
										if (copy_bright[i] >= 200)
											counter++;
									}
									if (counter == 0) {
										for (int i = 0; i < copy_bright.size(); i++)
											copy_bright[i]++;
									}
								}
								else {
									if (copy_bright[frame] < 200)
										copy_bright[frame]++;
								}
								update_screen = true;
								if (GetAsyncKeyState(VK_LCONTROL)) {
									while (GetAsyncKeyState(VK_F9))
										FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
								}
							}
							if (GetAsyncKeyState(VK_LMENU) && GetAsyncKeyState(VK_F8)) {	//decrease brightness
								if (GetAsyncKeyState(VK_RCONTROL)) {
									int counter = 0;
									for (int i = 0; i < copy_bright.size(); i++) {	//checking
										if (copy_bright[i] <= 0)
											counter++;
									}
									if (counter == 0) {
										for (int i = 0; i < copy_bright.size(); i++)
											copy_bright[i]--;
									}
								}
								else {
									if (copy_bright[frame] > 0)
										copy_bright[frame]--;
								}
								update_screen = true;
								if (GetAsyncKeyState(VK_LCONTROL)) {
									while (GetAsyncKeyState(VK_F8))
										FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
								}
							}

							if (prevframe_mode && update_screen && (frame > 0 || lock_frames)) {
								int prev = frame - 1;
								if (prev < 0 && lock_frames)
									prev = sprite.size() - 1;
								for (int i = 0; i < sprite[prev].size(); i++) {	//redrawing (previous frame)
									if (sprite[prev][i][0] != 0) {
										int counter = 0;
										for (int l = 0; l < sprite[frame].size(); l++) {
											if (sprite[frame][l][0] != 0 && sprite[prev][i][1] == sprite[frame][l][1] && sprite[prev][i][2] == sprite[frame][l][2])
												counter++;
										}
										for (int l = 0; l < copy_tmp.size(); l++) {
											if (copy_tmp[l][0] != 0 && sprite[prev][i][1] == copy_tmp[l][1] && sprite[prev][i][2] == copy_tmp[l][2])
												counter++;
										}
										if (counter == 0)
											printf("\033[?25l\033[38;2;%d;%d;%dm\033[48;2;%d;%d;%dm\033[%d;%dH%c\033[0m", bright_fx(sprite[prev][i][3], bright[prev]) * prevframe_op / 100, bright_fx(sprite[prev][i][4], bright[prev]) * prevframe_op / 100, bright_fx(sprite[prev][i][5], bright[prev]) * prevframe_op / 100, bright_fx(sprite[prev][i][6], bright[prev]) * prevframe_op / 100, bright_fx(sprite[prev][i][7], bright[prev]) * prevframe_op / 100, bright_fx(sprite[prev][i][8], bright[prev]) * prevframe_op / 100, sprite[prev][i][2], sprite[prev][i][1], char(sprite[prev][i][0]));
									}
								}
							}
							for (int i = 0; i < sprite[frame].size(); i++) {
								int counter = 0;
								for (int l = 0; l < copy_tmp.size(); l++) {
									if (sprite[frame][i][1] == copy_tmp[l][1] && sprite[frame][i][2] == copy_tmp[l][2] && copy_tmp[l][0] != 0)
										counter++;
								}
								if (sprite[frame][i][0] != 0 && counter == 0)
									printf("\033[?25l\033[38;2;%d;%d;%dm\033[48;2;%d;%d;%dm\033[%d;%dH%c\033[0m", bright_fx(sprite[frame][i][3], bright[frame]), bright_fx(sprite[frame][i][4], bright[frame]), bright_fx(sprite[frame][i][5], bright[frame]), bright_fx(sprite[frame][i][6], bright[frame]), bright_fx(sprite[frame][i][7], bright[frame]), bright_fx(sprite[frame][i][8], bright[frame]), sprite[frame][i][2], sprite[frame][i][1], char(sprite[frame][i][0]));
							}
							for (int i = 0; i < copy_tmp.size(); i++) {
								if (copy_tmp[i][0] != 0 && copy_tmp[i][1] >= 1 && copy_tmp[i][1] <= width && copy_tmp[i][2] >= 1 && copy_tmp[i][2] <= height)
									printf("\033[?25l\033[38;2;%d;%d;%dm\033[48;2;%d;%d;%dm\033[%d;%dH%c\033[0m", bright_fx(copy_tmp[i][3], copy_bright[frame] * bright[frame] / 100), bright_fx(copy_tmp[i][4], copy_bright[frame] * bright[frame] / 100), bright_fx(copy_tmp[i][5], copy_bright[frame] * bright[frame] / 100), bright_fx(copy_tmp[i][6], copy_bright[frame] * bright[frame] / 100), bright_fx(copy_tmp[i][7], copy_bright[frame] * bright[frame] / 100), bright_fx(copy_tmp[i][8], copy_bright[frame] * bright[frame] / 100), copy_tmp[i][2], copy_tmp[i][1], char(copy_tmp[i][0]));
							}

							if (GetAsyncKeyState(VK_ESCAPE) || GetAsyncKeyState(VK_BACK)) {	//cancel
								paste = false;
								break;
							}

							if (GetAsyncKeyState('M')) {	//mirror copy sprite
								for (int i = 0; i < copy_tmp.size(); i++) {	//clear
									if (copy_tmp[i][0] != 0 && copy_tmp[i][1] >= 1 && copy_tmp[i][1] <= width && copy_tmp[i][2] >= 1 && copy_tmp[i][2] <= height)
										printf("\033[%d;%dH ", copy_tmp[i][2], copy_tmp[i][1]);
								}
								if (GetAsyncKeyState(VK_LCONTROL)) {
									for (int px = 0; px < copy_tmp.size(); px++)
										copy_tmp[px][2] = height - copy_tmp[px][2] + 1;
								}
								else {
									for (int px = 0; px < copy_tmp.size(); px++)
										copy_tmp[px][1] = width - copy_tmp[px][1] + 1;
								}

								while (GetAsyncKeyState('M') || GetAsyncKeyState(VK_LCONTROL))
									FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
							}

							if (GetAsyncKeyState(VK_F5)) {	//refresh
								if (GetAsyncKeyState(VK_LCONTROL))
									console_config(width, xtr_height, f_sizeY, f_sizeX, FW_NORMAL, 1, L"Terminal");
								system("cls");
							}

							if (GetAsyncKeyState(VK_UP)) {	//copy sprite slip up
								y_slip--;
								while (GetAsyncKeyState(VK_UP))
									FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
							}
							if (GetAsyncKeyState(VK_LEFT)) {	//copy sprite slip left
								x_slip--;
								while (GetAsyncKeyState(VK_LEFT))
									FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
							}
							if (GetAsyncKeyState(VK_DOWN)) {	//copy sprite slip down
								y_slip++;
								while (GetAsyncKeyState(VK_DOWN))
									FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
							}
							if (GetAsyncKeyState(VK_RIGHT)) {	//copy sprite slip right
								x_slip++;
								while (GetAsyncKeyState(VK_RIGHT))
									FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
							}

							sleep_for(speed);
						}
					}
					if (paste) {
						for (int i = 0; i < copy_tmp.size(); i++) {
							for (int l = 3; l <= 8; l++)
								copy_tmp[i][l] = bright_fx(copy_tmp[i][l], copy_bright[frame]);
							if (copy_tmp[i][1] >= 1 && copy_tmp[i][1] <= width && copy_tmp[i][2] >= 1 && copy_tmp[i][2] <= height) {
								int counter = 0;
								for (int px = 0; px < sprite[frame].size(); px++) {
									if (sprite[frame][px][1] == copy_tmp[i][1] && sprite[frame][px][2] == copy_tmp[i][2] && (!is_null(sprite[frame][px], 9, 11) || sprite[frame][px][0] == 0 || copy_tmp[i][0] != 0 || !is_null(copy_tmp[i], 9, 11))) {
										if (!is_null(copy_tmp[i], 9, 11)) {
											counter++;
											for (int l = 9; l < 12; l++)
												sprite[frame][px][l] = copy_tmp[i][l];
										}
										if (!is_null(copy_tmp[i], 3, 8)) {
											counter++;
											sprite[frame][px][0] = copy_tmp[i][0];
											for (int c = 3; c <= 8; c++)
												sprite[frame][px][c] = copy_tmp[i][c];
										}
									}
								}
								if (counter == 0)
									sprite[frame].push_back(copy_tmp[i]);
							}
						}
					}
					while (GetAsyncKeyState('Z') || GetAsyncKeyState(VK_SPACE) || GetAsyncKeyState(VK_MULTIPLY) || GetAsyncKeyState(VK_DIVIDE) || GetAsyncKeyState('V') || GetAsyncKeyState('C') || GetAsyncKeyState(VK_END) || GetAsyncKeyState(VK_INSERT) || GetAsyncKeyState(VK_LBUTTON) || GetAsyncKeyState(VK_RETURN) || GetAsyncKeyState(VK_ESCAPE) || GetAsyncKeyState(VK_BACK) || GetAsyncKeyState(VK_LSHIFT) || GetAsyncKeyState(VK_RSHIFT) || GetAsyncKeyState(VK_RMENU) || GetAsyncKeyState(VK_DELETE) || GetAsyncKeyState(VK_F8) || GetAsyncKeyState(VK_F9) || GetAsyncKeyState(VK_F10) || GetAsyncKeyState(VK_F12))
						FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
					system("cls");
				}
			}

			if (GetAsyncKeyState(VK_MBUTTON) && !GetAsyncKeyState(VK_LMENU) && ((incdec == 0 && !config_mode) || config_mode)) {	//get symbol/conf
				if (config_mode) {
					int get_conf = get_n(sprite, frame, x, y, 8 + conf[0], conf[1]);
					if (get_conf != 0)
						conf[1] = get_conf;
				}
				else {
					int get_sym = get_n(sprite, frame, x, y, 0, sym);
					if (get_sym != 0)
						sym = get_sym;
				}
			}
			if (GetAsyncKeyState(VK_MBUTTON) && !GetAsyncKeyState(VK_LMENU) && !config_mode && incdec == 1) {	//get clr (pipette)
				if (GetAsyncKeyState(VK_LCONTROL)) {
					for (int c = 0; c < 3; c++)
						clr_b[c] = bright_fx(get_nclr(sprite, frame, x, y, 6 + c, clr_b[c]), bright[frame]);
				}
				else {
					for (int c = 0; c < 3; c++)
						clr_f[c] = bright_fx(get_nclr(sprite, frame, x, y, 3 + c, clr_f[c]), bright[frame]);
				}
			}
			if (GetAsyncKeyState(VK_F6) && !config_mode) {	//get clr (hex)
				if (GetAsyncKeyState(VK_LCONTROL))
					get_clr(clr_b);
				else
					get_clr(clr_f);
			}

			if (GetAsyncKeyState(VK_F10)) {	//configuration mode
				incdec = 0;
				if (config_mode)
					config_mode = false;
				else
					config_mode = true;

				while (GetAsyncKeyState(VK_F10) || GetAsyncKeyState(VK_RETURN) || GetAsyncKeyState(VK_BACK))
					FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				system("cls");
				update_screen = true;
			}

			if (GetAsyncKeyState(VK_F12) && !config_mode) {	//previous frame mode
				if (prevframe_mode)
					prevframe_mode = false;
				else
					prevframe_mode = true;
				while (GetAsyncKeyState(VK_F12))
					FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				system("cls");
				update_screen = true;
			}

			if (GetAsyncKeyState(VK_SPACE) && !GetAsyncKeyState(VK_CONTROL)) {	//preview
				console_config(width, height, f_sizeY, f_sizeX, FW_NORMAL, 0, L"Terminal", L"preview");
				preview(sprite, delay, audio, bright);
				update_screen = true;
			}

			if (GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState('S') && !(GetAsyncKeyState(VK_LSHIFT) || GetAsyncKeyState(VK_RSHIFT))) {
				sprite_menu(sprite, delay, audio, bright, sprite_name, sprite_title, width, height, f_sizeY, f_sizeX, frame, true);
				while (GetAsyncKeyState(VK_LCONTROL) || GetAsyncKeyState(VK_RCONTROL) || GetAsyncKeyState('S'))
					FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				update_screen = true;
			}
			if (GetAsyncKeyState(VK_ESCAPE)) {	//sprite menu
				sprite_menu(sprite, delay, audio, bright, sprite_name, sprite_title, width, height, f_sizeY, f_sizeX, frame);
				while (GetAsyncKeyState('Z') || GetAsyncKeyState(VK_MULTIPLY) || GetAsyncKeyState(VK_DIVIDE) || GetAsyncKeyState('V') || GetAsyncKeyState('C') || GetAsyncKeyState(VK_TAB) || GetAsyncKeyState(VK_END) || GetAsyncKeyState(VK_INSERT) || GetAsyncKeyState(VK_SPACE) || GetAsyncKeyState(VK_ESCAPE) || GetAsyncKeyState(VK_RETURN) || GetAsyncKeyState(VK_BACK) || GetAsyncKeyState(VK_CONTROL) || GetAsyncKeyState(VK_RMENU) || GetAsyncKeyState(VK_DELETE) || GetAsyncKeyState(VK_LSHIFT) || GetAsyncKeyState(VK_RSHIFT) || GetAsyncKeyState(VK_F8) || GetAsyncKeyState(VK_F9) || GetAsyncKeyState(VK_F10) || GetAsyncKeyState(VK_F12))	//flushing tab/space/esc/enter/.. buffer
					FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				system("cls");
				console_config(width, xtr_height, f_sizeY, f_sizeX, FW_NORMAL, 1, L"Terminal");
				update_screen = true;
			}
		}

		cleanup(sprite, width, height);
		if (prevframe_mode && update_screen && !config_mode && (frame > 0 || lock_frames)) {
			int prev = frame - 1;
			if (prev < 0 && lock_frames)
				prev = sprite.size() - 1;
			for (int i = 0; i < sprite[prev].size(); i++) {	//redrawing (previous frame)
				if (sprite[prev][i][0] != 0) {
					int counter = 0;
					for (int l = 0; l < sprite[frame].size(); l++) {
						if (sprite[prev][i][1] == sprite[frame][l][1] && sprite[prev][i][2] == sprite[frame][l][2] && sprite[frame][l][0] != 0)
							counter++;
					}
					if (counter == 0)
						printf("\033[?25l\033[38;2;%d;%d;%dm\033[48;2;%d;%d;%dm\033[%d;%dH%c\033[0m", bright_fx(sprite[prev][i][3], bright[prev]) * prevframe_op / 100, bright_fx(sprite[prev][i][4], bright[prev]) * prevframe_op / 100, bright_fx(sprite[prev][i][5], bright[prev]) * prevframe_op / 100, bright_fx(sprite[prev][i][6], bright[prev]) * prevframe_op / 100, bright_fx(sprite[prev][i][7], bright[prev]) * prevframe_op / 100, bright_fx(sprite[prev][i][8], bright[prev]) * prevframe_op / 100, sprite[prev][i][2], sprite[prev][i][1], char(sprite[prev][i][0]));
				}
			}
		}
		if (sprite_change != sprite || update_screen) {
			if (!config_mode) {
				for (int i = 0; i < sprite[frame].size(); i++) {	//redrawing
					if (sprite[frame][i][0] != 0) {
						printf("\033[?25l\033[38;2;%d;%d;%dm\033[48;2;%d;%d;%dm\033[%d;%dH%c\033[0m", bright_fx(sprite[frame][i][3], bright[frame]), bright_fx(sprite[frame][i][4], bright[frame]), bright_fx(sprite[frame][i][5], bright[frame]), bright_fx(sprite[frame][i][6], bright[frame]), bright_fx(sprite[frame][i][7], bright[frame]), bright_fx(sprite[frame][i][8], bright[frame]), sprite[frame][i][2], sprite[frame][i][1], char(sprite[frame][i][0]));
					}
				}
			}
			else {
				for (int i = 0; i < sprite[frame].size(); i++) {	//redrawing (config_mode)
					if (sprite[frame][i][8 + conf[0]] != 0) {
						int tmp_conf[] = { conf[0], sprite[frame][i][8 + conf[0]] };
						printf("\033[?25l\033[%d;%dH%s", sprite[frame][i][2], sprite[frame][i][1], get_conf0(tmp_conf).c_str());
					}
				}
			}
			update_screen = false;
		}
	}
}