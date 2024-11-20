#include "icb_gui.h"
#include <stdlib.h>
#include <time.h>
#include <windows.h>

int keypressed;
int FRM1;


struct Game {
    ICBYTES m;
    struct Player {
        int boxX = 10;
        int boxY = 380;
        int bulletX, bulletY;
        bool bulletActive = false;
        int bulletCooldown = 0;
    } player;

    struct Box {
        int x, y = 0;
        int width;
        int k;
    } fallingBox;

    struct Fragment {
        int x, y;
        int dx, dy;
        bool active;
    } fragments[15];

    bool isShattered = false;
    int shatterDuration = 20;
} gameState;

void ICGUI_Create() {
    ICG_MWTitle("Shooting Falling Boxes Game");
    ICG_MWSize(450, 500);
    srand(time(NULL));
}

void InitializeShatter(Game& game, int direction) {
    for (int i = 0; i < 15; ++i) {
        game.fragments[i].x = game.fallingBox.x + (i % 3) * 7;
        game.fragments[i].y = game.fallingBox.y + (i / 3) * 7;

        if (direction == 1) {
            game.fragments[i].dx = rand() % 5 + 1;
            game.fragments[i].dy = (rand() % 3) - 1;
        }
        else if (direction == -1) {
            game.fragments[i].dx = -(rand() % 5 + 1);
            game.fragments[i].dy = (rand() % 3) - 1;
        }
        else {
            game.fragments[i].dx = (rand() % 3) - 1;
            game.fragments[i].dy = -(rand() % 7 + 2);
        }

        game.fragments[i].active = true;
    }
    game.isShattered = true;
    game.shatterDuration = 20;
}



void ClearFragments(Game& game) {
    for (auto& fragment : game.fragments) {
        if (fragment.active) {
            FillRect(game.m, fragment.x, fragment.y, 5, 5, 0x000000);
            fragment.active = false;
        }
    }
}
void HandleBulletCollision(Game& game) {
    int bulletPosRelativeToBox = game.player.bulletX - game.fallingBox.x;

    if (bulletPosRelativeToBox >= 0 && bulletPosRelativeToBox <= game.fallingBox.width) {

        if (bulletPosRelativeToBox < 3 * game.fallingBox.k) {
            InitializeShatter(game, 1);
        }

        else if (bulletPosRelativeToBox > game.fallingBox.width - 3 * game.fallingBox.k) {
            InitializeShatter(game, -1);
        }

        else {
            InitializeShatter(game, 0);
        }


        game.isShattered = true;
        game.player.bulletActive = false;
    }
}

void ResetFallingBox(Game& game) {
    game.fallingBox.k = rand() % 5 + 1;
    game.fallingBox.width = 10 * game.fallingBox.k;
    game.fallingBox.x = rand() % (450 - game.fallingBox.width);
    game.fallingBox.y = 0;
    game.isShattered = false;
}

void UpdateShatter(Game& game) {
    bool anyActive = false;
    for (auto& fragment : game.fragments) {
        if (fragment.active) {
            FillRect(game.m, fragment.x, fragment.y, 5, 5, 0x000000);
            fragment.x += fragment.dx;
            fragment.y += fragment.dy;
            FillRect(game.m, fragment.x, fragment.y, 5, 5, 0xffd700);

            if (fragment.x < 0 || fragment.x > 450 || fragment.y < 0 || fragment.y > 400) {
                fragment.active = false;
            }
            else {
                anyActive = true;
            }
        }
    }

    if (--game.shatterDuration <= 0 || !anyActive) {
        ClearFragments(game);
        game.isShattered = false;
        ResetFallingBox(game);
    }
}

DWORD WINAPI FallingBox(LPVOID lpParam) {
    Game& game = (Game)lpParam;
    while (true) {

        FillRect(game.m, game.fallingBox.x, game.fallingBox.y, game.fallingBox.width, game.fallingBox.width, 0x000000);

        if (game.isShattered) {
            UpdateShatter(game);
        }
        else {
            game.fallingBox.y += 5;
            if (game.fallingBox.y >= 380) {
                ResetFallingBox(game);
            }
        }

        if (!game.isShattered) {
            FillRect(game.m, game.fallingBox.x, game.fallingBox.y, game.fallingBox.width, game.fallingBox.width, 0x0000ff);
        }
        DisplayImage(FRM1, game.m);
        Sleep(30);
    }
    return 0;
}

DWORD WINAPI BulletControl(LPVOID lpParam) {
    Game& game = (Game)lpParam;
    while (true) {
        if (game.player.bulletActive) {
            FillRect(game.m, game.player.bulletX, game.player.bulletY, 5, 10, 0x000000);
            game.player.bulletY -= 5;

            if (game.player.bulletY < 0) {
                game.player.bulletActive = false;
            }
            if (game.player.bulletActive &&
                game.player.bulletY <= game.fallingBox.y + game.fallingBox.width &&
                game.player.bulletY >= game.fallingBox.y) {
                HandleBulletCollision(game);
            }
            if (game.player.bulletActive) {
                FillRect(game.m, game.player.bulletX, game.player.bulletY, 5, 10, 0x00ff00);
            }
        }
        if (game.player.bulletCooldown > 0) {
            game.player.bulletCooldown--;
        }
        Sleep(30);
    }
    return 0;
}

DWORD WINAPI SlidingBox(LPVOID lpParam) {
    Game& game = (Game)lpParam;
    while (true) {
        FillRect(game.m, game.player.boxX, game.player.boxY, 20, 6, 0x000000);
        if (keypressed == 37 && game.player.boxX > 0) game.player.boxX -= 5;
        if (keypressed == 39 && game.player.boxX < 430) game.player.boxX += 5;
        FillRect(game.m, game.player.boxX, game.player.boxY, 20, 6, 0xff0000);
        DisplayImage(FRM1, game.m);
        Sleep(30);
    }
    return 0;
}

void WhenKeyPressed(int k) {
    keypressed = k;
    if (k == 32 && gameState.player.bulletCooldown <= 0 && !gameState.player.bulletActive) {
        gameState.player.bulletX = gameState.player.boxX + 10;
        gameState.player.bulletY = gameState.player.boxY - 10;
        gameState.player.bulletActive = true;
        gameState.player.bulletCooldown = 5;
    }
}

void butonfonk() {
    static HANDLE hThreadSlidingBox = NULL;
    static HANDLE hThreadFallingBox = NULL;
    static HANDLE hThreadBullet = NULL;

    if (!hThreadSlidingBox && !hThreadFallingBox && !hThreadBullet) {
        hThreadSlidingBox = CreateThread(NULL, 0, SlidingBox, &gameState, 0, NULL);
        hThreadFallingBox = CreateThread(NULL, 0, FallingBox, &gameState, 0, NULL);
        hThreadBullet = CreateThread(NULL, 0, BulletControl, &gameState, 0, NULL);

        SetFocus(ICG_GetMainWindow());
    }
}

void ICGUI_main() {
    ICG_Button(5, 5, 120, 25, "Start / Stop", butonfonk);
    FRM1 = ICG_FrameMedium(5, 40, 400, 400);
    ICG_SetOnKeyPressed(WhenKeyPressed);

    ResetFallingBox(gameState);

    CreateImage(gameState.m, 400, 400, ICB_UINT);
}