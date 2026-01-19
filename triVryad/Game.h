#ifndef GAME_H
#define GAME_H

#include <vector>
#include <windows.h>
#include <gdiplus.h>
#include "Gem.h"
#include "Constants.h"
#include "IGameObserver.h"
#include <mutex>
#include <random>
class Game {
private:
	std::mt19937 rng;
	int GetGemImageSize() const;
	GemType FindSafeGemType(int row, int col);
	Gem* GenerateGemAboveGrid(int col);
	void RefillColumnAboveGrid(int col);
	std::vector<std::vector<Gem*>> grid;
	POINT selected;
	ULONG_PTR gdiplusToken;
	std::vector<Gdiplus::Image*> gemImages;
	std::vector<IGameObserver*> observers;

	mutable std::mutex gameStateMtx;
	GameState currentState;

	std::vector<Gem*> GenerateRandomGems();
	void DistributeGemsOnGrid(std::vector<Gem*>& gems);
	void ApplyGravity();
	void FindAllMatches(std::vector<std::pair<int, int>>& matchedGems);
	bool CheckHorizontalMatch(int row, int col, std::vector<std::pair<int, int>>& matchedGems);
	bool CheckVerticalMatch(int row, int col, std::vector<std::pair<int, int>>& matchedGems);
	void RemoveMatchedGems(const std::vector<std::pair<int, int>>& matchedGems);
	bool HasMovingGems() const;
	Gem* GenerateRandomGem(int row, int col);

public:
	Game();
	~Game();
	void InitializeGDI();
	void InitializeGrid();
	void LoadGemImages();
	void DrawGrid(HDC hdc, int windowWidth, int windowHeight);
	void HandleClick(int x, int y, int cellWidth, int cellHeight);
	void Update(int windowWidth, int windowHeight);
	void Cleanup();
	bool IsFirstGemSelected() const;
	bool CanPlayerMove() const;
	Gdiplus::Image* GetGemImage(GemType type) const;
	GameState GetCurrentState() const;

	void AddObserver(IGameObserver* observer) {
		std::lock_guard<std::mutex> lock(gameStateMtx);
		observers.push_back(observer);
		OutputDebugString(L"obs in list.\n");
	}

	void RemoveObserver(IGameObserver* observer) {
		std::lock_guard<std::mutex> lock(gameStateMtx);
		observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
	}
	void NotifyObserversUnsafe() {
		for (auto observer : observers) {
			observer->OnAnimationCompleted();
		}
	}

	void NotifyObservers() {
		std::lock_guard<std::mutex> lock(gameStateMtx);
		for (auto observer : observers) {
			observer->OnAnimationCompleted();
		}
	}

	Gem* GetGemAt(int row, int col) const {
		if (row >= 0 && row < GRID_SIZE && col >= 0 && col < GRID_SIZE) {
			return grid[row][col];
		}
		return nullptr;
	}

	void RemoveGemAt(int row, int col) {
		if (row >= 0 && row < GRID_SIZE && col >= 0 && col < GRID_SIZE) {
			if (grid[row][col]) {
				delete grid[row][col];
				grid[row][col] = nullptr;
			}
		}
	}
};

#endif