#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#define CLEAR_SCREEN "cls"
#else
#include <unistd.h>
#define CLEAR_SCREEN "clear"
#endif

// --- ANSI 컬러 코드 정의 ---
#define RESET   "\x1b[0m"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define BOLD    "\x1b[1m"

// 1. 구조체 설계: 플레이어 상태 관리
struct Player {
    int hp;
    int max_hp;
    int floor;
};

// 2. 배열 활용: 몬스터 데이터 목록
const char* monsterNames[4] = { "고블린", "스켈레톤", "슬라임", "흉폭한 드래곤" };
const char* monsterArts[4] = {
    GREEN "  (o_o)\n /(   )\\\n   ^ ^  " RESET,             // 고블린
    RESET "  [x_x]\n  /| |\\\n    ^ ^  " RESET,             // 스켈레톤
    CYAN  "   ___\n  (O_O)\n (_____) " RESET,             // 슬라임
    RED   "  \\|/ ____ \\|/\n   @~/ ,. \\~@\n  /_( \\__/ )_\\" RESET // 드래곤
};

// 함수 원형 선언
int loadHighScore();
void saveHighScore(int score);
void clearScreen();
int getChoice();
void waitForEnter();
void drawBar(const char* label, int current, int max, const char* color);
void showTitle(int bestScore);
void playGame(struct Player* p, int monster_hp, int monster_max_hp, int monster_idx);

int main() {
    srand((unsigned int)time(NULL));
    int choice;
    int bestScore = loadHighScore(); // 파일에서 최고 기록 로드

    while (1) {
        clearScreen();
        showTitle(bestScore);

        printf(CYAN "  [1] " RESET "탑의 문을 연다 (Game Start)\n");
        printf(CYAN "  [2] " RESET "도망친다 (Exit)\n");
        printf("\n  " BOLD "> 선택: " RESET);

        choice = getChoice(); // 예외 처리된 안전한 입력 받기

        if (choice == 1) {
            struct Player player = { 100, 100, 1 }; // 1층부터 시작

            clearScreen();
            printf(YELLOW "\n\n  [ 육중한 문이 열립니다. 횃불에 불이 켜집니다... ]\n\n" RESET);
#ifdef _WIN32
            Sleep(1500);
#else
            sleep(1);
#endif

            // 1층 몬스터 생성 후 게임 핵심 재귀 함수 진입
            int m_idx = rand() % 4;
            int m_hp = 40; // 1층 기본 몬스터 체력
            playGame(&player, m_hp, m_hp, m_idx);

            // 게임 오버 처리 (재귀 탈출 후 온 곳)
            clearScreen();
            printf(RED BOLD "\n\n  [ YOU DIED ]\n" RESET);
            printf("  최종 도달 층수: " YELLOW "%d층\n" RESET, player.floor);

            if (player.floor > bestScore) {
                printf(MAGENTA BOLD "  ★ NEW HIGHSCORE! 최고 기록을 갱신했습니다! ★\n" RESET);
                saveHighScore(player.floor);
                bestScore = player.floor;
            }
            waitForEnter();
        }
        else if (choice == 2) {
            printf(YELLOW "\n  당신은 등을 돌려 도망쳤습니다...\n" RESET);
            break;
        }
    }
    return 0;
}

// 3. 파일 입출력: 데이터 세이브/로드 함수
int loadHighScore() {
    FILE* file = fopen("highscore.dat", "r");
    if (file == NULL) return 1; // 파일 없으면 1층이 최고 기록
    int score = 1;
    fscanf(file, "%d", &score);
    fclose(file);
    return score;
}

void saveHighScore(int score) {
    FILE* file = fopen("highscore.dat", "w");
    if (file != NULL) {
        fprintf(file, "%d", score);
        fclose(file);
    }
}

// 콘솔 제어 관련 유틸리티 함수
void clearScreen() {
    system(CLEAR_SCREEN);
}

// ★버그 수정★ 문자를 입력해도 버퍼를 청소하여 무한 루프를 방지하는 안전한 입력 함수
int getChoice() {
    int choice;
    if (scanf("%d", &choice) != 1) {
        while (getchar() != '\n'); // 입력 버퍼에 남아있는 문자 찌꺼기 싹 비우기
        return -1; // 잘못된 입력 코드 반환
    }
    while (getchar() != '\n'); // 정상 입력 후 남아있는 엔터('\n') 제거
    return choice;
}

// ★버그 수정★ 숫자 상관없이 그냥 엔터만 쳐도 다음으로 넘어가는 깔끔한 일시정지 함수
void waitForEnter() {
    printf(BOLD "\n  [ 엔터 키(Enter)를 누르면 계속합니다 ]" RESET);
    while (getchar() != '\n');
}

// 대기업 스타일 HP 게이지 바 출력 함수 (플레이어와 몬스터 공용)
void drawBar(const char* label, int current, int max, const char* color) {
    printf("  %s: [", label);
    int barWidth = 15;
    int fill = (current * barWidth) / max;
    if (fill < 0) fill = 0;
    if (fill > barWidth) fill = barWidth;

    for (int i = 0; i < barWidth; ++i) {
        if (i < fill) printf("%s■" RESET, color);
        else printf(" ");
    }
    printf("] %d/%d\n", current, max);
}

void showTitle(int bestScore) {
    printf(CYAN BOLD);
    printf("  ==========================================\n");
    printf("         T O W E R   O F   E S C A P E      \n");
    printf("  ==========================================\n" RESET);
    printf("        최고 기록: " YELLOW "%d F" RESET "\n\n", bestScore);
}

// ★핵심 로직 수정★ 턴 제어를 위한 100% 재귀 알고리즘 함수
void playGame(struct Player* p, int monster_hp, int monster_max_hp, int monster_idx) {
    // [기저 조건 1] 플레이어가 죽으면 게임 오버 및 재귀 즉시 종료
    if (p->hp <= 0) {
        return;
    }

    // [기저 조건 2] 몬스터를 처치했을 때 -> 다음 층 세팅 후 재귀 호출로 전진!
    if (monster_hp <= 0) {
        clearScreen();
        printf(GREEN BOLD "\n\n  ★ %s(을)를 물리쳤습니다! ★\n" RESET, monsterNames[monster_idx]);
        printf("  탑의 계단을 올라 다음 층으로 이동합니다.\n");
        waitForEnter();

        p->floor += 1; // 층수 증가
        int next_idx = rand() % 4; // 다음 몬스터 선택
        int next_max_hp = 40 + (p->floor * 15); // 층이 올라갈수록 몬스터 피통 증가

        // 다음 층으로 이동하는 재귀 호출
        playGame(p, next_max_hp, next_max_hp, next_idx);
        return;
    }

    // --- 현재 층 전투 화면 출력 (몬스터가 살아있을 때 대기 루프) ---
    clearScreen();
    printf(MAGENTA BOLD "  --- [ %d F ] ----------------------------\n" RESET, p->floor);
    drawBar("PLAYER   ", p->hp, p->max_hp, GREEN);
    drawBar("MONSTER  ", monster_hp, monster_max_hp, RED);
    printf(MAGENTA "  ------------------------------------------\n" RESET);

    printf("\n%s\n\n", monsterArts[monster_idx]);
    printf("  야생의 [" YELLOW "%s" RESET "]이(가) 당신의 앞을 가로막고 있습니다!\n\n", monsterNames[monster_idx]);

    printf(CYAN "  [1] " RESET "공격한다 (적에게 피해를 주고 반격을 받음)\n");
    printf(CYAN "  [2] " RESET "단단히 방어하며 휴식 (확률적 회복 또는 기습 허용)\n");
    printf("\n  " BOLD "> 행동 선택: " RESET);

    int choice = getChoice();
    int next_monster_hp = monster_hp;

    if (choice == 1) {
        // 공격 로직
        int p_dmg = (rand() % 11) + 12; // 플레이어 데미지 12~22
        int m_dmg = (rand() % 10) + 5 + (p->floor * 2); // 몬스터 데미지 (층당 강해짐)

        printf(YELLOW "\n  >> 당신의 맹공! [%s]에게 %d의 피해를 입혔습니다!\n" RESET, monsterNames[monster_idx], p_dmg);
        next_monster_hp -= p_dmg;

        // 몬스터가 아직 살아있다면 반격
        if (next_monster_hp > 0) {
            printf(RED "  >> [%s]의 반격! 당신은 %d의 피해를 입었습니다.\n" RESET, monsterNames[monster_idx], m_dmg);
            p->hp -= m_dmg;
        }
        waitForEnter();
    }
    else if (choice == 2) {
        // 방어/휴식 로직 (성공하면 회복하고 끝, 실패하면 적에게만 맞음)
        if (rand() % 2 == 0) {
            int heal = (rand() % 16) + 15; // 15~30 회복
            printf(GREEN "\n  >> 방어 성공! 숨을 고르며 체력을 %d 회복했습니다.\n" RESET, heal);
            p->hp += heal;
            if (p->hp > p->max_hp) p->hp = p->max_hp;
        }
        else {
            int m_dmg = (rand() % 11) + 10 + (p->floor * 2); // 기습은 더 아픔
            printf(RED "\n  >> 방어 실패! 빈틈을 보여 [%s]에게 %d의 강력한 기습을 당했습니다!\n" RESET, monsterNames[monster_idx], m_dmg);
            p->hp -= m_dmg;
        }
        waitForEnter();
    }
    else {
        printf(YELLOW "\n  >> 잘못된 입력으로 어물쩡거리다가 선제 공격을 당했습니다!\n" RESET);
        p->hp -= 10;
        waitForEnter();
    }

    // [재귀 호출]: 전투가 끝나지 않았으므로 동일한 층, 변동된 체력을 가지고 다시 함수 호출
    playGame(p, next_monster_hp, monster_max_hp, monster_idx);
}
