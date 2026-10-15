#include <Windows.h>
#include <vector>

const int screenWidth = 1280;
const int screenHeight = 720;

const int playerSize = 50;
const int playerSpeed = 5;

const int bulletSize = 10;
const int bulletSpeed = 10;

const int enemySize = 40;
const int fireInterval = 200;
const DWORD enemyRespawnTime = 3000;

int playerX = 200;
int playerY = 300;

bool keyUp = false;
bool keyDown = false;
bool keyLeft = false;
bool keyRight = false;
bool spaceDown = false;

bool enemyAlive = true;

DWORD lastFireTime = 0;
DWORD enemyDefeatedTime = 0;

struct Bullet {
	int x;
	int y;
};

struct Enemy {
	int x;
	int y;
};

std::vector<Bullet> bullets;
std::vector<Enemy> enemies;

void InitializeEnemies() {
	enemies.clear();
	enemies.push_back({1000, 300});
	enemyAlive = true;
}

void FireBullet() { bullets.push_back({playerX + playerSize, playerY + playerSize / 2 - bulletSize / 2}); }

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_UP:
		case 'W':
			keyUp = true;
			break;

		case VK_DOWN:
		case 'S':
			keyDown = true;
			break;

		case VK_LEFT:
		case 'A':
			keyLeft = true;
			break;

		case VK_RIGHT:
		case 'D':
			keyRight = true;
			break;

		case VK_SPACE:
			if (!spaceDown) {
				spaceDown = true;
				FireBullet();
				lastFireTime = GetTickCount();
			}
			break;
		}
		return 0;

	case WM_KEYUP:
		switch (wParam) {
		case VK_UP:
		case 'W':
			keyUp = false;
			break;

		case VK_DOWN:
		case 'S':
			keyDown = false;
			break;

		case VK_LEFT:
		case 'A':
			keyLeft = false;
			break;

		case VK_RIGHT:
		case 'D':
			keyRight = false;
			break;

		case VK_SPACE:
			spaceDown = false;
			break;
		}
		return 0;

	case WM_TIMER: {
		// 自機の移動
		if (keyUp) {
			playerY -= playerSpeed;
		}
		if (keyDown) {
			playerY += playerSpeed;
		}
		if (keyLeft) {
			playerX -= playerSpeed;
		}
		if (keyRight) {
			playerX += playerSpeed;
		}

		// 画面外に出ないようにする
		if (playerX < 0) {
			playerX = 0;
		}
		if (playerY < 0) {
			playerY = 0;
		}
		if (playerX > screenWidth - playerSize) {
			playerX = screenWidth - playerSize;
		}
		if (playerY > screenHeight - playerSize) {
			playerY = screenHeight - playerSize;
		}

		// スペースキー長押しで連射
		DWORD currentTime = GetTickCount();

		if (spaceDown && currentTime - lastFireTime >= fireInterval) {
			FireBullet();
			lastFireTime = currentTime;
		}

		// 弾を移動
		for (auto& bullet : bullets) {
			bullet.x += bulletSpeed;
		}

		// 画面外の弾を削除
		for (auto bullet = bullets.begin(); bullet != bullets.end();) {
			if (bullet->x > screenWidth) {
				bullet = bullets.erase(bullet);
			} else {
				++bullet;
			}
		}

		// 弾と敵の当たり判定
		for (auto bullet = bullets.begin(); bullet != bullets.end();) {

			bool hit = false;

			for (auto enemy = enemies.begin(); enemy != enemies.end(); ++enemy) {

				if (bullet->x < enemy->x + enemySize && bullet->x + bulletSize > enemy->x && bullet->y < enemy->y + enemySize && bullet->y + bulletSize > enemy->y) {

					enemies.erase(enemy);

					enemyAlive = false;
					enemyDefeatedTime = GetTickCount();

					hit = true;
					break;
				}
			}

			if (hit) {
				bullet = bullets.erase(bullet);
			} else {
				++bullet;
			}
		}

		// 敵を3秒後に復活させる
		if (!enemyAlive && GetTickCount() - enemyDefeatedTime >= enemyRespawnTime) {

			enemies.push_back({1000, 300});
			enemyAlive = true;
		}

		InvalidateRect(hwnd, nullptr, FALSE);
		return 0;
	}

	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		// 背景
		HBRUSH backgroundBrush = CreateSolidBrush(RGB(211, 211, 211));
		FillRect(hdc, &ps.rcPaint, backgroundBrush);
		DeleteObject(backgroundBrush);

		// 白い自機と弾
		HBRUSH whiteBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
		HPEN whitePen = (HPEN)GetStockObject(WHITE_PEN);

		HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, whiteBrush);
		HPEN oldPen = (HPEN)SelectObject(hdc, whitePen);

		Ellipse(hdc, playerX, playerY, playerX + playerSize, playerY + playerSize);

		for (const auto& bullet : bullets) {
			Ellipse(hdc, bullet.x, bullet.y, bullet.x + bulletSize, bullet.y + bulletSize);
		}

		SelectObject(hdc, oldBrush);
		SelectObject(hdc, oldPen);

		// 赤い敵
		if (enemyAlive) {
			HBRUSH enemyBrush = CreateSolidBrush(RGB(255, 0, 0));
			HPEN enemyPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));

			oldBrush = (HBRUSH)SelectObject(hdc, enemyBrush);
			oldPen = (HPEN)SelectObject(hdc, enemyPen);

			for (const auto& enemy : enemies) {
				Ellipse(hdc, enemy.x, enemy.y, enemy.x + enemySize, enemy.y + enemySize);
			}

			SelectObject(hdc, oldBrush);
			SelectObject(hdc, oldPen);

			DeleteObject(enemyBrush);
			DeleteObject(enemyPen);
		}

		EndPaint(hwnd, &ps);
		return 0;
	}

	case WM_DESTROY:
		KillTimer(hwnd, 1);
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
	const wchar_t CLASS_NAME[] = L"GameWindow";

	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	RegisterClass(&wc);

	RECT rect = {0, 0, screenWidth, screenHeight};
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

	HWND hwnd =
	    CreateWindowEx(0, CLASS_NAME, L"Shooting Game", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, hInstance, nullptr);

	if (hwnd == nullptr) {
		return 0;
	}

	InitializeEnemies();

	SetTimer(hwnd, 1, 16, nullptr);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	MSG msg = {};

	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}