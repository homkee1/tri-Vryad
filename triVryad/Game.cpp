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

int GRID_SIZE = 16;
int CELL_SIZE = 50;
int GEM_TYPES_COUNT = 3;
double SpeedAnimationA4ChangePos = 30.0;
std::wstring PNG_FOLDER_PATH = L"";
int RENDERING_QUALITY = 2;
int GEM_IMAGE_SIZE = 128;

Gdiplus::Bitmap* LoadAndResizePNG(const std::wstring& filePath, int width, int height) {
	Gdiplus::Bitmap* originalBitmap = Gdiplus::Bitmap::FromFile(filePath.c_str());
	if (!originalBitmap || originalBitmap->GetLastStatus() != Gdiplus::Ok) {
		std::wstring errMsg = L"не удалось загрузить изображение:\n" + filePath;
		MessageBoxW(NULL, errMsg.c_str(), L"errGDI+", MB_ICONERROR);
		delete originalBitmap;
		return nullptr;
	}

	Gdiplus::Bitmap* resizedBitmap = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);
	if (!resizedBitmap || resizedBitmap->GetLastStatus() != Gdiplus::Ok) {
		delete originalBitmap;
		delete resizedBitmap;
		return nullptr;
	}

	Gdiplus::Graphics graphics(resizedBitmap);

	if (RENDERING_QUALITY == 0) {
		graphics.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
		graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighSpeed);
		graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighSpeed);
	}
	else if (RENDERING_QUALITY == 1) {
		graphics.SetInterpolationMode(Gdiplus::InterpolationModeBilinear);
		graphics.SetSmoothingMode(Gdiplus::SmoothingModeNone);
		graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeNone);
	}
	else {
		graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
		graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
		graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
	}

	graphics.DrawImage(originalBitmap, 0, 0, width, height);

	delete originalBitmap;
	return resizedBitmap;
}

Game::Game()
	: selected{ -1, -1 },
	currentState(GameState::IDLE),
	rng(std::random_device{}())
{
	grid.resize(GRID_SIZE);
	for (int i = 0; i < GRID_SIZE; ++i) {
		grid[i].resize(GRID_SIZE);
	}
	LoadGemImages();
}

Game::~Game() {
	for (auto image : gemImages) {
		delete image;
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
	std::wstring pngFolder = PNG_FOLDER_PATH;
	if (pngFolder.empty()) {
		wchar_t exePath[MAX_PATH];
		GetModuleFileNameW(NULL, exePath, MAX_PATH);
		std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();
		pngFolder = (exeDir / L"png").wstring();
	}

	std::vector<std::wstring> pngFiles;
	FindPngFiles(pngFolder, pngFiles);


	std::random_device rd;
	std::mt19937 g(rd());
	if (!pngFiles.empty()) {
		std::shuffle(pngFiles.begin(), pngFiles.end(), g);
	}

	int gemSize = GetGemImageSize();
	int loadedCount = (int)pngFiles.size() < GEM_TYPES_COUNT ? (int)pngFiles.size() : GEM_TYPES_COUNT;

	for (int i = 0; i < loadedCount; ++i) {
		Gdiplus::Bitmap* image = LoadAndResizePNG(pngFiles[i].c_str(), gemSize, gemSize);
		if (image) {
			if (image->GetLastStatus() == Gdiplus::Ok) {
				gemImages.push_back(image);
			}
			else {
				delete image;
			}
		}
	}
}

std::vector<std::unique_ptr<Gem>> Game::GenerateRandomGems() {
	std::vector<std::unique_ptr<Gem>> gems;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> typeDist(0, GEM_TYPES_COUNT - 1);

	int totalMinGems = GEM_TYPES_COUNT * MinimalGemsForType;
	if (totalMinGems > GRID_SIZE * GRID_SIZE) {
		MessageBox(NULL, L"totalMinGems > GRID_SIZE * GRID_SIZE!", L"err", MB_ICONERROR);
		return gems;
	}

	int totalMaxGems = GEM_TYPES_COUNT * MaximumGemsForType;
	if (totalMaxGems < GRID_SIZE * GRID_SIZE) {
		MessageBox(NULL, L"totalMaxGems < GRID_SIZE * GRID_SIZE!", L"err", MB_ICONERROR);
		return gems;
	}

	for (int i = 0; i < GEM_TYPES_COUNT; ++i) {
		for (int j = 0; j < MinimalGemsForType; ++j) {
			GemType type = static_cast<GemType>(i);
			Gdiplus::Color color = GetColorForType(i);
			gems.push_back(std::make_unique<Gem>(0, 0, color, type));
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
		Gdiplus::Color color = GetColorForType(type);
		gems.push_back(std::make_unique<Gem>(0, 0, color, gemType));

		gemCounts[type]++;
	}

	std::shuffle(gems.begin(), gems.end(), gen);
	return gems;
}

void Game::DistributeGemsOnGrid(std::vector<std::unique_ptr<Gem>>& gems) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::shuffle(gems.begin(), gems.end(), gen);

	int index = 0;
	for (int i = 0; i < GRID_SIZE; ++i) {
		for (int j = 0; j < GRID_SIZE; ++j) {
			if (index < gems.size()) {
				grid[i][j] = std::move(gems[index++]);
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
			Gdiplus::Color color = GetColorForType(static_cast<int>(gemType));
			grid[row][col] = std::make_unique<Gem>(col, row, color, gemType);
		}
	}

	for (int col = 0; col < GRID_SIZE; ++col) {
		for (int row = GRID_SIZE - 1; row >= 0; --row) {
			if (grid[row][col]) {
				GemType gemType = grid[row][col]->getType();
				Gdiplus::Color color = GetColorForType(static_cast<int>(gemType));

				int startYAboveGrid = row - GRID_SIZE;
				auto newGem = std::make_unique<Gem>(col, startYAboveGrid, color, gemType);

				LARGE_INTEGER startTime;
				QueryPerformanceCounter(&startTime);

				newGem->SetTargetPos(col, row, startTime);
				grid[row][col] = std::move(newGem);
			}
		}
	}
}

std::unique_ptr<Gem> Game::GenerateRandomGem(int row, int col) {
	GemType gemType = FindSafeGemType(row, col);
	Gdiplus::Color color = GetColorForType(static_cast<int>(gemType));
	return std::make_unique<Gem>(col, row, color, gemType);
}

void Game::DrawGrid(HDC hdc, int windowWidth, int windowHeight) {
	Gdiplus::Graphics graphics(hdc);
	Gdiplus::SolidBrush blackBrush(Gdiplus::Color(255, 0, 0, 0));
	graphics.FillRectangle(&blackBrush, 0, 0, windowWidth, windowHeight);

	if (RENDERING_QUALITY == 0) {
		graphics.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
		graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighSpeed);
		graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighSpeed);
	}
	else if (RENDERING_QUALITY == 1) {
		graphics.SetInterpolationMode(Gdiplus::InterpolationModeBilinear);
		graphics.SetSmoothingMode(Gdiplus::SmoothingModeNone);
		graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeNone);
	}
	else {
		graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
		graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
		graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
	}

	int cellWidth = windowWidth / GRID_SIZE;
	int cellHeight = windowHeight / GRID_SIZE;

	POINT cursorPos;
	GetCursorPos(&cursorPos);

	HWND hMainWnd = FindWindowW(L"GemClass", NULL);
	if (!hMainWnd) {
		hMainWnd = GetActiveWindow();
	}

	ScreenToClient(hMainWnd, &cursorPos);

	int cursorX = cursorPos.x / cellWidth;
	int cursorY = cursorPos.y / cellHeight;

	for (int i = 0; i < GRID_SIZE; ++i) {
		for (int j = 0; j < GRID_SIZE; ++j) {
			if (grid[i][j]) {
				POINT gemPos = grid[i][j]->getPos();
				int scaledX = gemPos.x * cellWidth / CELL_SIZE;
				int scaledY = gemPos.y * cellHeight / CELL_SIZE;

				Gdiplus::Image* image = GetGemImage(grid[i][j]->getType());

				grid[i][j]->Draw(graphics, image, cellWidth, cellHeight);

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

				std::swap(grid[selected.y][selected.x], grid[y][x]);
				currentState = GameState::SWAPPING;

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
						grid[i][j] = std::move(grid[k][j]);
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

	Gem* gem1 = grid[row][col].get();
	Gem* gem2 = grid[row][col + 1].get();
	Gem* gem3 = grid[row][col + 2].get();

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

	Gem* gem1 = grid[row][col].get();
	Gem* gem2 = grid[row + 1][col].get();
	Gem* gem3 = grid[row + 2][col].get();

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
						grid[row][col] = std::move(grid[k][col]);
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

		for (size_t i = 0; i < emptyRows.size(); ++i) {
			int row = emptyRows[i];
			GemType gemType = FindSafeGemType(row, col);
			Gdiplus::Color color = GetColorForType(static_cast<int>(gemType));

			int startYAboveGrid = -(int)emptyRows.size() + i;
			auto newGem = std::make_unique<Gem>(col, startYAboveGrid, color, gemType);
			grid[row][col] = std::move(newGem);

			LARGE_INTEGER startTime;
			QueryPerformanceCounter(&startTime);

			grid[row][col]->SetTargetPos(col, row, startTime);
		}
	}
}

void Game::Update(int windowWidth, int windowHeight) {
	for (int i = 0; i < GRID_SIZE; ++i) {
		for (int j = 0; j < GRID_SIZE; ++j) {
			if (grid[i][j] && grid[i][j]->IsMoving()) {
				grid[i][j]->Update();
			}
		}
	}

	bool triggerNotify = false;

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
			triggerNotify = true;
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
				triggerNotify = true;
			}
		}
		break;
	}

	default:
		break;
	}

	if (triggerNotify) {
		NotifyObservers();
	}
}

bool Game::IsFirstGemSelected() const {
	return (selected.x != -1 && selected.y != -1);
}

bool Game::CanPlayerMove() const {
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
	return currentState;
}

std::unique_ptr<Gem> Game::GenerateGemAboveGrid(int col) {
	GemType gemType = FindSafeGemType(-1, col);
	Gdiplus::Color color = GetColorForType(static_cast<int>(gemType));
	return std::make_unique<Gem>(col, -1, color, gemType);
}

void Game::RefillColumnAboveGrid(int col) {
	for (int row = 0; row < GRID_SIZE; ++row) {
		if (!grid[row][col]) {
			auto newGem = GenerateGemAboveGrid(col);
			grid[row][col] = std::move(newGem);

			LARGE_INTEGER startTime;
			QueryPerformanceCounter(&startTime);

			grid[row][col]->SetTargetPos(col, row, startTime);
			return;
		}
	}
}

int Game::GetGemImageSize() const {
	return GEM_IMAGE_SIZE;
}

void Game::Reset() {
	for (size_t i = 0; i < grid.size(); ++i) {
		for (size_t j = 0; j < grid[i].size(); ++j) {
			grid[i][j].reset();
		}
	}

	for (auto image : gemImages) {
		delete image;
	}
	gemImages.clear();

	grid.clear();
	grid.resize(GRID_SIZE);
	for (int i = 0; i < GRID_SIZE; ++i) {
		grid[i].resize(GRID_SIZE);
	}

	selected = { -1, -1 };
	currentState = GameState::IDLE;

	LoadGemImages();
	InitializeGrid();
}