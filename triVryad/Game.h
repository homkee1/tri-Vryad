#ifndef GAME_H
#define GAME_H

#include <vector>
#include <windows.h>
#include <gdiplus.h>
#include <memory>
#include <random>
#include <algorithm>
#include "Gem.h"
#include "Constants.h"
#include "IGameObserver.h"

class Game {
private:
	std::mt19937 rng;
	int GetGemImageSize() const;
	GemType FindSafeGemType(int row, int col);
	std::unique_ptr<Gem> GenerateGemAboveGrid(int col);
	void RefillColumnAboveGrid(int col);
	std::vector<std::vector<std::unique_ptr<Gem>>> grid;
	POINT selected;
	std::vector<Gdiplus::Image*> gemImages;
	std::vector<IGameObserver*> observers;

	GameState currentState;

	std::vector<std::unique_ptr<Gem>> GenerateRandomGems();
	void DistributeGemsOnGrid(std::vector<std::unique_ptr<Gem>>& gems);
	void ApplyGravity();
	void FindAllMatches(std::vector<std::pair<int, int>>& matchedGems);
	bool CheckHorizontalMatch(int row, int col, std::vector<std::pair<int, int>>& matchedGems);
	bool CheckVerticalMatch(int row, int col, std::vector<std::pair<int, int>>& matchedGems);
	void RemoveMatchedGems(const std::vector<std::pair<int, int>>& matchedGems);
	bool HasMovingGems() const;
	std::unique_ptr<Gem> GenerateRandomGem(int row, int col);

public:
	Game();
	~Game();
	void InitializeGrid();
	void LoadGemImages();
	void DrawGrid(HDC hdc, int windowWidth, int windowHeight);
	void HandleClick(int x, int y, int cellWidth, int cellHeight);
	void Update(int windowWidth, int windowHeight);
	void Reset();
	bool IsFirstGemSelected() const;
	bool CanPlayerMove() const;
	Gdiplus::Image* GetGemImage(GemType type) const;
	GameState GetCurrentState() const;

	void AddObserver(IGameObserver* observer) {
		observers.push_back(observer);
		OutputDebugString(L"obs in list.\n");
	}

	void RemoveObserver(IGameObserver* observer) {
		observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
	}

	void NotifyObservers() {
		for (auto observer : observers) {
			observer->OnAnimationCompleted();
		}
	}

	Gem* GetGemAt(int row, int col) const {
		if (row >= 0 && row < GRID_SIZE && col >= 0 && col < GRID_SIZE) {
			return grid[row][col].get();
		}
		return nullptr;
	}

	void RemoveGemAt(int row, int col) {
		if (row >= 0 && row < GRID_SIZE && col >= 0 && col < GRID_SIZE) {
			grid[row][col].reset();
		}
	}
};

#endif