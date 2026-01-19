#include "Game.h"
#include "Gem.h"
#include "Constants.h"
#include <ctime>
#include <Windows.h>
#include <gdiplus.h>
#include <fstream>
#include <comdef.h>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <filesystem>
#include <set>
#include <map>
namespace fs = std::filesystem;

void Game::InitializeGDI() {
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::Status status = Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	if (status != Gdiplus::Ok) {
		MessageBox(NULL, L"GDI+", L"Error", MB_ICONERROR);
	}
}

Gdiplus::Bitmap* LoadAndResizePNG(const std::wstring& filePath, int width, int height) {
	Gdiplus::Bitmap* originalBitmap = Gdiplus::Bitmap::FromFile(filePath.c_str());
	if (!originalBitmap || originalBitmap->GetLastStatus() != Gdiplus::Ok) {
		MessageBox(NULL, L"image", L"Error", MB_ICONERROR);
		delete originalBitmap;
		return nullptr;
	}

	Gdiplus::Bitmap* resizedBitmap = (Gdiplus::Bitmap*)originalBitmap->GetThumbnailImage(width, height, nullptr, nullptr);
	if (!resizedBitmap || resizedBitmap->GetLastStatus() != Gdiplus::Ok) {
		MessageBox(NULL, L"image", L"Error", MB_ICONERROR);
		delete originalBitmap;
		return nullptr;
	}

	delete originalBitmap;
	return resizedBitmap;
}


Game::Game()
	: grid(GRID_SIZE, std::vector<Gem*>(GRID_SIZE, nullptr)),
	selected{ -1, -1 },
	gdiplusToken(0),
	currentState(GameState::IDLE),
	rng(std::random_device{}())
{
	InitializeGDI();
	LoadGemImages();
}

Game::~Game() {
	for (auto image : gemImages) {
		delete image;
	}
	for (int i = 0; i < GRID_SIZE; ++i) {
		for (int j = 0; j < GRID_SIZE; ++j) {
			if (grid[i][j]) {
				delete grid[i][j];
			}
		}
	}
}

void FindPngFiles(const std::wstring& folderPath, std::vector<std::wstring>& pngFiles) {
	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile((folderPath + L"\\*").c_str(), &findFileData);

	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}

	do {
		std::wstring fileName = findFileData.cFileName;
		if (fileName != L"." && fileName != L"..") {
			std::wstring fullPath = folderPath + L"\\" + fileName;

			if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				FindPngFiles(fullPath, pngFiles);
			}
			else if (fileName.size() >= 4 && fileName.substr(fileName.size() - 4) == L".png") {
				pngFiles.push_back(fullPath);
			}
		}
	} while (FindNextFile(hFind, &findFileData) != 0);

	FindClose(hFind);
}

void Game::LoadGemImages() {
	wchar_t exePath[MAX_PATH];
	GetModuleFileNameW(NULL, exePath, MAX_PATH);
	std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();
	std::filesystem::path pngFolder = exeDir / L"png";
	std::vector<std::wstring> pngFiles;
	FindPngFiles(pngFolder.wstring(), pngFiles);
	if (pngFiles.size() < GEM_TYPES_COUNT) {
		MessageBox(NULL, L"в папке меньше файлов чем требуется", L"Error", MB_ICONERROR);
		return;
	}

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(pngFiles.begin(), pngFiles.end(), g);

	int gemSize = GetGemImageSize();

	for (int i = 0; i < GEM_TYPES_COUNT; ++i) {
		Gdiplus::Bitmap* image = LoadAndResizePNG(pngFiles[i].c_str(), gemSize, gemSize);
		if (image && image->GetLastStatus() == Gdiplus::Ok) {
			gemImages.push_back(image);
		}
		else {
			MessageBox(NULL, L"nopngload", L"Error", MB_ICONERROR);
		}
	}
}


std::vector<Gem*> Game::GenerateRandomGems() {
	std::vector<Gem*> gems;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> typeDist(0, GEM_TYPES_COUNT - 1);

	int totalMinGems = GEM_TYPES_COUNT * MinimalGemsForType;
	if (totalMinGems > GRID_SIZE * GRID_SIZE) {
		MessageBox(NULL, L"totalMinGems > GRID_SIZE * GRID_SIZE!", L"Ошибка", MB_ICONERROR);
		return gems;
	}

	int totalMaxGems = GEM_TYPES_COUNT * MaximumGemsForType;
	if (totalMaxGems < GRID_SIZE * GRID_SIZE) {
		MessageBox(NULL, L"totalMaxGems < GRID_SIZE * GRID_SIZE!", L"Ошибка", MB_ICONERROR);
		return gems;
	}

	for (int i = 0; i < GEM_TYPES_COUNT; ++i) {
		for (int j = 0; j < MinimalGemsForType; ++j) {
			GemType type = static_cast<GemType>(i);
			Gdiplus::Color color = COLORS[i];
			gems.push_back(new Gem(0, 0, color, type));
		}
	}

	int remainingGems = (GRID_SIZE * GRID_SIZE) - (GEM_TYPES_COUNT * MinimalGemsForType);
	std::vector<int> gemCounts(GEM_TYPES_COUNT, MinimalGemsForType);

	for (int i = 0; i < remainingGems; ++i) {
		int type;
		do {
			type = typeDist(gen);
		} while (gemCounts[type] >= MaximumGemsForType);

		GemType gemType = static_cast<GemType>(type);
		Gdiplus::Color color = COLORS[type];
		gems.push_back(new Gem(0, 0, color, gemType));

		gemCounts[type]++;
	}

	std::shuffle(gems.begin(), gems.end(), gen);
	return gems;
}

void Game::DistributeGemsOnGrid(std::vector<Gem*>& gems) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::shuffle(gems.begin(), gems.end(), gen);

	int index = 0;
	for (int i = 0; i < GRID_SIZE; ++i) {
		for (int j = 0; j < GRID_SIZE; ++j) {
			if (index < gems.size()) {
				grid[i][j] = gems[index++];
				grid[i][j]->ChangePos(j, i);
			}
			else {
				grid[i][j] = nullptr;
			}
		}
	}
}

GemType Game::FindSafeGemType(int row, int col) {
	std::set<GemType> forbiddenTypes;


	if (col >= 2) {
		if (grid[row][col - 1] && grid[row][col - 2] &&
			grid[row][col - 1]->getType() == grid[row][col - 2]->getType()) {
			forbiddenTypes.insert(grid[row][col - 1]->getType());
		}
	}

	if (col >= 1 && col < GRID_SIZE - 1) {
		if (grid[row][col - 1] && grid[row][col + 1] &&
			grid[row][col - 1]->getType() == grid[row][col + 1]->getType()) {
			forbiddenTypes.insert(grid[row][col - 1]->getType());
		}
	}

	if (col < GRID_SIZE - 2) {
		if (grid[row][col + 1] && grid[row][col + 2] &&
			grid[row][col + 1]->getType() == grid[row][col + 2]->getType()) {
			forbiddenTypes.insert(grid[row][col + 1]->getType());
		}
	}


	if (row >= 2) {
		if (grid[row - 1][col] && grid[row - 2][col] &&
			grid[row - 1][col]->getType() == grid[row - 2][col]->getType()) {
			forbiddenTypes.insert(grid[row - 1][col]->getType());
		}
	}

	if (row >= 1 && row < GRID_SIZE - 1) {
		if (grid[row - 1][col] && grid[row + 1][col] &&
			grid[row - 1][col]->getType() == grid[row + 1][col]->getType()) {
			forbiddenTypes.insert(grid[row - 1][col]->getType());
		}
	}

	if (row < GRID_SIZE - 2) {
		if (grid[row + 1][col] && grid[row + 2][col] &&
			grid[row + 1][col]->getType() == grid[row + 2][col]->getType()) {
			forbiddenTypes.insert(grid[row + 1][col]->getType());
		}
	}


	std::vector<GemType> candidates;
	for (int i = 0; i < GEM_TYPES_COUNT; ++i) {
		GemType type = static_cast<GemType>(i);
		if (forbiddenTypes.find(type) == forbiddenTypes.end()) {
			candidates.push_back(type);
		}
	}


	if (!candidates.empty()) {
		std::uniform_int_distribution<> dist(0, (int)candidates.size() - 1);
		return candidates[dist(rng)];  
	}


	std::map<GemType, int> conflictCount;
	for (int i = 0; i < GEM_TYPES_COUNT; ++i) {
		conflictCount[static_cast<GemType>(i)] = 0;
	}

	for (GemType type : forbiddenTypes) {
		conflictCount[type]++;
	}

	GemType bestType = static_cast<GemType>(0);
	int minConflicts = INT_MAX;
	for (const auto& pair : conflictCount) {
		if (pair.second < minConflicts) {
			minConflicts = pair.second;
			bestType = pair.first;
		}
	}

	return bestType;
}

void Game::InitializeGrid() {

	for (int row = 0; row < GRID_SIZE; ++row) {
		for (int col = 0; col < GRID_SIZE; ++col) {
			GemType gemType = FindSafeGemType(row, col);
			Gdiplus::Color color = COLORS[static_cast<int>(gemType)];
			grid[row][col] = new Gem(col, row, color, gemType);
		}
	}

	for (int col = 0; col < GRID_SIZE; ++col) {
		for (int row = GRID_SIZE - 1; row >= 0; --row) {
			if (grid[row][col]) {
				GemType gemType = grid[row][col]->getType();
				Gdiplus::Color color = COLORS[static_cast<int>(gemType)];

				int startYAboveGrid = row - GRID_SIZE;
				Gem* newGem = new Gem(col, startYAboveGrid, color, gemType);

				LARGE_INTEGER startTime;
				QueryPerformanceCounter(&startTime);

				newGem->SetTargetPos(col, row, startTime);
				delete grid[row][col];
				grid[row][col] = newGem;
			}
		}
	}
}


Gem* Game::GenerateRandomGem(int row, int col) {
	GemType gemType = FindSafeGemType(row, col);
	Gdiplus::Color color = COLORS[static_cast<int>(gemType)];
	Gem* newGem = new Gem(col, row, color, gemType);
	return newGem;
}


void Game::DrawGrid(HDC hdc, int windowWidth, int windowHeight) {
	Gdiplus::Graphics graphics(hdc);
	Gdiplus::SolidBrush blackBrush(Gdiplus::Color(255, 0, 0, 0));
	graphics.FillRectangle(&blackBrush, 0, 0, windowWidth, windowHeight);

	int cellWidth = windowWidth / GRID_SIZE;
	int cellHeight = windowHeight / GRID_SIZE;

	POINT cursorPos;
	GetCursorPos(&cursorPos);
	ScreenToClient(GetActiveWindow(), &cursorPos);

	int cursorX = cursorPos.x / cellWidth;
	int cursorY = cursorPos.y / cellHeight;

	for (int i = 0; i < GRID_SIZE; ++i) {
		for (int j = 0; j < GRID_SIZE; ++j) {
			if (grid[i][j]) {
				POINT gemPos = grid[i][j]->getPos();
				int scaledX = gemPos.x * cellWidth / CELL_SIZE;
				int scaledY = gemPos.y * cellHeight / CELL_SIZE;

				Gdiplus::Image* image = GetGemImage(grid[i][j]->getType());

				if (image && image->GetLastStatus() == Gdiplus::Ok) {
					graphics.DrawImage(image, scaledX, scaledY, cellWidth, cellHeight);
				}

				bool isCursorOver = (cursorX == j && cursorY == i);

				if (selected.x == j && selected.y == i) {
					Gdiplus::Pen whitePen(Gdiplus::Color(255, 255, 255, 255), 3);
					graphics.DrawRectangle(&whitePen, scaledX, scaledY, cellWidth, cellHeight);
				}
				else if (isCursorOver && !this->IsFirstGemSelected()) {
					Gdiplus::Pen greenPen(Gdiplus::Color(255, 0, 255, 0), 3);
					graphics.DrawRectangle(&greenPen, scaledX, scaledY, cellWidth, cellHeight);
				}
				else if (isCursorOver && this->IsFirstGemSelected()) {
					int dx = abs(selected.x - j);
					int dy = abs(selected.y - i);

					if ((dx == 1 && dy == 0) || (dx == 0 && dy == 1)) {
						Gdiplus::Pen greenPen(Gdiplus::Color(255, 0, 255, 0), 3);
						graphics.DrawRectangle(&greenPen, scaledX, scaledY, cellWidth, cellHeight);
					}
					else {
						Gdiplus::Pen redPen(Gdiplus::Color(255, 255, 0, 0), 3);
						graphics.DrawRectangle(&redPen, scaledX, scaledY, cellWidth, cellHeight);
					}
				}
			}
		}
	}
}

void Game::HandleClick(int pixelX, int pixelY, int cellWidth, int cellHeight) {
	if (!CanPlayerMove()) {
		return;
	}

	int x = pixelX / cellWidth;
	int y = pixelY / cellHeight;

	if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE) {
		if (!grid[y][x]) {
			return;
		}

		if (selected.x == -1 && selected.y == -1) {
			selected.x = x;
			selected.y = y;
		}
		else {
			int dx = abs(selected.x - x);
			int dy = abs(selected.y - y);

			if ((dx == 1 && dy == 0) || (dx == 0 && dy == 1)) {
				LARGE_INTEGER startTime;
				QueryPerformanceCounter(&startTime);

				grid[selected.y][selected.x]->SetTargetPos(x, y, startTime);
				grid[y][x]->SetTargetPos(selected.x, selected.y, startTime);

				Gem* temp = grid[selected.y][selected.x];
				grid[selected.y][selected.x] = grid[y][x];
				grid[y][x] = temp;

				{
					std::lock_guard<std::mutex> lock(gameStateMtx);
					currentState = GameState::SWAPPING;
				}

				selected.x = -1;
				selected.y = -1;
			}
			else {
				selected.x = -1;
				selected.y = -1;
			}
		}
	}
}

bool Game::HasMovingGems() const {
	for (int i = 0; i < GRID_SIZE; ++i) {
		for (int j = 0; j < GRID_SIZE; ++j) {
			if (grid[i][j] && grid[i][j]->IsMoving()) {
				return true;
			}
		}
	}
	return false;
}

void Game::ApplyGravity() {
	for (int j = 0; j < GRID_SIZE; ++j) {
		for (int i = GRID_SIZE - 1; i >= 0; --i) {
			if (!grid[i][j]) {
				for (int k = i - 1; k >= 0; --k) {
					if (grid[k][j]) {
						grid[i][j] = grid[k][j];
						grid[k][j] = nullptr;

						LARGE_INTEGER startTime;
						QueryPerformanceCounter(&startTime);
						grid[i][j]->SetTargetPos(j, i, startTime);
						break;
					}
				}
			}
		}
	}
}

bool Game::CheckHorizontalMatch(int row, int col, std::vector<std::pair<int, int>>& matchedGems) {
	if (col + 2 >= GRID_SIZE) return false;

	Gem* gem1 = grid[row][col];
	Gem* gem2 = grid[row][col + 1];
	Gem* gem3 = grid[row][col + 2];

	if (gem1 && gem2 && gem3 &&
		gem1->getType() == gem2->getType() &&
		gem2->getType() == gem3->getType()) {

		if (std::find(matchedGems.begin(), matchedGems.end(), std::make_pair(row, col)) == matchedGems.end()) {
			matchedGems.push_back({ row, col });
		}
		if (std::find(matchedGems.begin(), matchedGems.end(), std::make_pair(row, col + 1)) == matchedGems.end()) {
			matchedGems.push_back({ row, col + 1 });
		}
		if (std::find(matchedGems.begin(), matchedGems.end(), std::make_pair(row, col + 2)) == matchedGems.end()) {
			matchedGems.push_back({ row, col + 2 });
		}
		return true;
	}
	return false;
}

bool Game::CheckVerticalMatch(int row, int col, std::vector<std::pair<int, int>>& matchedGems) {
	if (row + 2 >= GRID_SIZE) return false;

	Gem* gem1 = grid[row][col];
	Gem* gem2 = grid[row + 1][col];
	Gem* gem3 = grid[row + 2][col];

	if (gem1 && gem2 && gem3 &&
		gem1->getType() == gem2->getType() &&
		gem2->getType() == gem3->getType()) {

		if (std::find(matchedGems.begin(), matchedGems.end(), std::make_pair(row, col)) == matchedGems.end()) {
			matchedGems.push_back({ row, col });
		}
		if (std::find(matchedGems.begin(), matchedGems.end(), std::make_pair(row + 1, col)) == matchedGems.end()) {
			matchedGems.push_back({ row + 1, col });
		}
		if (std::find(matchedGems.begin(), matchedGems.end(), std::make_pair(row + 2, col)) == matchedGems.end()) {
			matchedGems.push_back({ row + 2, col });
		}
		return true;
	}
	return false;
}

void Game::FindAllMatches(std::vector<std::pair<int, int>>& matchedGems) {
	matchedGems.clear();

	for (int i = 0; i < GRID_SIZE; ++i) {
		for (int j = 0; j < GRID_SIZE; ++j) {
			CheckHorizontalMatch(i, j, matchedGems);
			CheckVerticalMatch(i, j, matchedGems);
		}
	}
}

void Game::RemoveMatchedGems(const std::vector<std::pair<int, int>>& matchedGems) {
	for (const auto& pos : matchedGems) {
		RemoveGemAt(pos.first, pos.second);
	}

	std::set<int> affectedColumns;
	for (const auto& pos : matchedGems) {
		affectedColumns.insert(pos.second);
	}

	for (int col : affectedColumns) {
		for (int row = GRID_SIZE - 1; row >= 0; --row) {
			if (!grid[row][col]) {
				for (int k = row - 1; k >= 0; --k) {
					if (grid[k][col]) {
						grid[row][col] = grid[k][col];
						grid[k][col] = nullptr;

						LARGE_INTEGER startTime;
						QueryPerformanceCounter(&startTime);
						grid[row][col]->SetTargetPos(col, row, startTime);
						break;
					}
				}
			}
		}
	}

	for (int col : affectedColumns) {
		std::vector<int> emptyRows;
		for (int row = 0; row < GRID_SIZE; ++row) {
			if (!grid[row][col]) {
				emptyRows.push_back(row);
			}
		}

		for (int i = 0; i < emptyRows.size(); ++i) {
			int row = emptyRows[i];

			GemType gemType = FindSafeGemType(row, col);
			Gdiplus::Color color = COLORS[static_cast<int>(gemType)];

			int startYAboveGrid = -(int)emptyRows.size() + i;
			Gem* newGem = new Gem(col, startYAboveGrid, color, gemType);
			grid[row][col] = newGem;

			LARGE_INTEGER startTime;
			QueryPerformanceCounter(&startTime);

			newGem->SetTargetPos(col, row, startTime);
		}
	}
}



void Game::Update(int windowWidth, int windowHeight) {
	for (int i = 0; i < GRID_SIZE; ++i) {
		for (int j = 0; j < GRID_SIZE; ++j) {
			if (grid[i][j] && grid[i][j]->IsMoving()) {
				grid[i][j]->Update(windowWidth, windowHeight);
			}
		}
	}

	std::lock_guard<std::mutex> lock(gameStateMtx);

	switch (currentState) {
	case GameState::IDLE:
		break;

	case GameState::SWAPPING: {
		if (!HasMovingGems()) {
			currentState = GameState::FALLING;
			ApplyGravity();
		}
		break;
	}

	case GameState::FALLING: {
		if (!HasMovingGems()) {
			currentState = GameState::MATCHING;
		}
		break;
	}

	case GameState::MATCHING: {
		std::vector<std::pair<int, int>> matchedGems;
		FindAllMatches(matchedGems);

		if (!matchedGems.empty()) {
			RemoveMatchedGems(matchedGems);
			currentState = GameState::REFILLING;
		}
		else {
			currentState = GameState::IDLE;
			NotifyObserversUnsafe();
		}
		break;
	}

	case GameState::REFILLING: {
		if (!HasMovingGems()) {
			std::vector<std::pair<int, int>> matchedGems;
			FindAllMatches(matchedGems);

			if (!matchedGems.empty()) {
				RemoveMatchedGems(matchedGems);
				currentState = GameState::REFILLING;
			}
			else {
				currentState = GameState::IDLE;
				NotifyObserversUnsafe();
			}
		}
		break;
	}

	default:
		break;
	}
}


void Game::Cleanup() {
	Gdiplus::GdiplusShutdown(gdiplusToken);
}

bool Game::IsFirstGemSelected() const {
	return (selected.x != -1 && selected.y != -1);
}

bool Game::CanPlayerMove() const {
	std::lock_guard<std::mutex> lock(gameStateMtx);
	return currentState == GameState::IDLE;
}

Gdiplus::Image* Game::GetGemImage(GemType type) const {
	int index = static_cast<int>(type);
	if (index >= 0 && index < gemImages.size()) {
		return gemImages[index];
	}
	return nullptr;
}

GameState Game::GetCurrentState() const {
	std::lock_guard<std::mutex> lock(gameStateMtx);
	return currentState;
}


Gem* Game::GenerateGemAboveGrid(int col) {
	GemType gemType = FindSafeGemType(-1, col);
	Gdiplus::Color color = COLORS[static_cast<int>(gemType)];

	Gem* newGem = new Gem(col, -1, color, gemType);
	return newGem;
}

void Game::RefillColumnAboveGrid(int col) {

	for (int row = 0; row < GRID_SIZE; ++row) {
		if (!grid[row][col]) {
			Gem* newGem = GenerateGemAboveGrid(col);
			grid[row][col] = newGem;

			LARGE_INTEGER startTime;
			QueryPerformanceCounter(&startTime);

			newGem->SetTargetPos(col, row, startTime);
			return;
		}
	}
}

int Game::GetGemImageSize() const {
	return (GRID_SIZE / 8) * 50;
}