#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <cstdlib>
#include <iomanip> 
#include <stack>
#include <sstream>
#include <chrono>
#include <fstream> // ��� ������ � �������
#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;
using namespace chrono;

const int GRID_SIZE = 9;

const string RESET_COLOR = "\033[0m";
const string GREEN_TEXT = "\033[32m";
const string RED_TEXT = "\033[31m";
const string BLUE_TEXT = "\033[34m";
const string YELLOW_TEXT = "\033[33m";

enum Difficulty { EASY, MEDIUM, HARD };

struct PlayerStats {
    string name;
    int wins;
    int losses;
    int totalGames;
    double averageGameTime;

    PlayerStats(string playerName) : name(playerName), wins(0), losses(0), totalGames(0), averageGameTime(0.0) {}

    void updateStats(bool win, double gameTime) {
        totalGames++;
        if (win) {
            wins++;
        }
        else {
            losses++;
        }
        averageGameTime = ((averageGameTime * (totalGames - 1)) + gameTime) / totalGames;
    }

    void displayStats() {
        cout << "Player: " << name << endl;
        cout << "Total Games: " << totalGames << endl;
        cout << "Wins: " << wins << endl;
        cout << "Losses: " << losses << endl;
        cout << "Average Game Time: " << fixed << setprecision(2) << averageGameTime << " seconds" << endl;
    }

    void saveStatsToFile(ofstream& outFile) {
        outFile << name << "," << wins << "," << losses << "," << totalGames << "," << fixed << setprecision(2) << averageGameTime << endl;
    }
};

// ������� ��� ���������� ���������� � ����
void savePlayerStatsToFile(vector<PlayerStats>& players) {
    ofstream outFile("player_stats.csv", ios::app);
    if (!outFile) {
        cout << "Error opening file!" << endl;
        return;
    }
    for (auto& player : players) {
        player.saveStatsToFile(outFile);
    }
    outFile.close();
}


// ������� ��� ����������� ����������� ���� � ���������� ����������
void displayGameResults(PlayerStats& player1, PlayerStats& player2, double gameTime) {
    cout << "Game Over!" << endl;

    bool player1Win = true;  // ������� �� ������ ����
    player1.updateStats(player1Win, gameTime);
    player2.updateStats(!player1Win, gameTime);

    player1.displayStats();
    player2.displayStats();

    vector<PlayerStats> players = { player1, player2 };
    savePlayerStatsToFile(players);
}

// �������� ���������� �� �����
vector<PlayerStats> loadStatsFromFile() {
    vector<PlayerStats> players;
    ifstream inFile("player_stats.csv");
    if (!inFile) {
        cout << "No saved stats found." << endl;
        return players;
    }

    string line;
    while (getline(inFile, line)) {
        stringstream ss(line);
        string name;
        int wins, losses, totalGames;
        double avgTime;
        getline(ss, name, ',');
        ss >> wins;
        ss.ignore(1, ',');
        ss >> losses;
        ss.ignore(1, ',');
        ss >> totalGames;
        ss.ignore(1, ',');
        ss >> avgTime;

        PlayerStats player(name);
        player.wins = wins;
        player.losses = losses;
        player.totalGames = totalGames;
        player.averageGameTime = avgTime;
        players.push_back(player);
    }
    inFile.close();
    return players;
}
struct Player {
    string name;
    int score;
};

struct Move {
    int row, col, value;
};

enum Language { ENGLISH, RUSSIAN };
enum Theme { DEFAULT, DARK, LIGHT };

class Sudoku {
private:
    int grid[GRID_SIZE][GRID_SIZE];
    int solution[GRID_SIZE][GRID_SIZE];
    stack<Move> moves;
    vector<Player> leaderboard;
    Language currentLanguage;
    Difficulty currentDifficulty;
    Theme currentTheme;
    time_point<steady_clock> startTime;
    time_point<steady_clock> player1StartTime, player2StartTime;
    int errorCount1, errorCount2;

    void enableANSIColors() {
#ifdef _WIN32
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
#endif
    }

    void showLocalTime() {
        time_t now = time(0);
        tm localtm;
        localtime_s(&localtm, &now);
        char buffer[26];
        asctime_s(buffer, sizeof(buffer), &localtm);
        cout << (currentLanguage == ENGLISH ? "Current Local Time: " : "������� ������� �����: ")
            << buffer;
    }

    void generatePuzzle() {
        int sampleGrid[GRID_SIZE][GRID_SIZE] = {
            {5, 3, 4, 6, 7, 8, 9, 1, 2},
            {6, 7, 2, 1, 9, 5, 3, 4, 8},
            {1, 9, 8, 3, 4, 2, 5, 6, 7},
            {8, 5, 9, 7, 6, 1, 4, 2, 3},
            {4, 2, 6, 8, 5, 3, 7, 9, 1},
            {7, 1, 3, 9, 2, 4, 8, 5, 6},
            {9, 6, 1, 5, 3, 7, 2, 8, 4},
            {2, 8, 7, 4, 1, 9, 6, 3, 5},
            {3, 4, 5, 2, 8, 6, 1, 7, 9}
        };

        copy(&sampleGrid[0][0], &sampleGrid[0][0] + GRID_SIZE * GRID_SIZE, &solution[0][0]);

        int cellsToHide = currentDifficulty == EASY ? 25 : currentDifficulty == MEDIUM ? 40 : 55;

        srand(static_cast<unsigned int>(time(0)));

        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                grid[i][j] = solution[i][j];
            }
        }

        while (cellsToHide > 0) {
            int row = rand() % GRID_SIZE;
            int col = rand() % GRID_SIZE;
            if (grid[row][col] != 0) {
                grid[row][col] = 0;
                cellsToHide--;
            }
        }
    }

    void generateDailyPuzzle() {
        time_t now = time(0);
        tm ltm;
        localtime_s(&ltm, &now);

        srand(static_cast<unsigned int>(ltm.tm_mday + ltm.tm_mon * 100 + ltm.tm_year * 10000));

        int sampleGrid[GRID_SIZE][GRID_SIZE] = {
            {5, 3, 4, 6, 7, 8, 9, 1, 2},
            {6, 7, 2, 1, 9, 5, 3, 4, 8},
            {1, 9, 8, 3, 4, 2, 5, 6, 7},
            {8, 5, 9, 7, 6, 1, 4, 2, 3},
            {4, 2, 6, 8, 5, 3, 7, 9, 1},
            {7, 1, 3, 9, 2, 4, 8, 5, 6},
            {9, 6, 1, 5, 3, 7, 2, 8, 4},
            {2, 8, 7, 4, 1, 9, 6, 3, 5},
            {3, 4, 5, 2, 8, 6, 1, 7, 9}
        };

        copy(&sampleGrid[0][0], &sampleGrid[0][0] + GRID_SIZE * GRID_SIZE, &solution[0][0]);

        int cellsToHide = currentDifficulty == EASY ? 25 : currentDifficulty == MEDIUM ? 40 : 55;

        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                grid[i][j] = solution[i][j];
            }
        }

        while (cellsToHide > 0) {
            int row = rand() % GRID_SIZE;
            int col = rand() % GRID_SIZE;
            if (grid[row][col] != 0) {
                grid[row][col] = 0;
                cellsToHide--;
            }
        }
    }

    void displayGrid() {
        cout << (currentLanguage == ENGLISH ? "\nSudoku Board:\n" : "\n���� ������:\n");
        cout << " -----------------------\n";
        for (int i = 0; i < GRID_SIZE; i++) {
            cout << "| ";
            for (int j = 0; j < GRID_SIZE; j++) {
                if (grid[i][j] == 0) {
                    cout << ". ";
                }
                else if (grid[i][j] == solution[i][j]) {
                    cout << YELLOW_TEXT << grid[i][j] << RESET_COLOR << " ";
                }
                else {
                    cout << grid[i][j] << " ";
                }
                if ((j + 1) % 3 == 0) cout << "| ";
            }
            cout << "\n";
            if ((i + 1) % 3 == 0) cout << " -----------------------\n";
        }
    }

    bool isValidMove(int row, int col, int num) {
        for (int x = 0; x < GRID_SIZE; x++) {
            if (grid[row][x] == num || grid[x][col] == num) return false;
        }
        int startRow = row - row % 3, startCol = col - col % 3;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                if (grid[i + startRow][j + startCol] == num) return false;
        return true;
    }

    void undoLastMove() {
        if (!moves.empty()) {
            Move lastMove = moves.top();
            moves.pop();
            grid[lastMove.row][lastMove.col] = 0;
            cout << GREEN_TEXT << (currentLanguage == ENGLISH ? "Last move undone.\n" : "��������� ��� �������.\n") << RESET_COLOR;
        }
        else {
            cout << RED_TEXT << (currentLanguage == ENGLISH ? "No moves to undo.\n" : "��� ����� ��� ������.\n") << RESET_COLOR;
        }
    }

    void showHint() {
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                if (grid[i][j] == 0) {
                    cout << BLUE_TEXT << (currentLanguage == ENGLISH ? "Hint: Try placing " : "���������: ���������� ��������� ")
                        << solution[i][j] << (currentLanguage == ENGLISH ? " at (" : " �� (") << i + 1 << ", " << j + 1 << ")\n" << RESET_COLOR;
                    return;
                }
            }
        }
        cout << RED_TEXT << (currentLanguage == ENGLISH ? "No hints available.\n" : "��������� ������ ���.\n") << RESET_COLOR;
    }

    void handleInvalidMove(int playerTurn) {
        if (playerTurn == 1) {
            errorCount1++;
            if (errorCount1 >= 5) {
                cout << RED_TEXT << (currentLanguage == ENGLISH ? "Player 1 has reached the maximum number of errors. Game over!\n"
                    : "����� 1 ������ ������������� ���������� ������. ���� ��������!\n") << RESET_COLOR;
                exit(0);
            }
        }
        else {
            errorCount2++;
            if (errorCount2 >= 5) {
                cout << RED_TEXT << (currentLanguage == ENGLISH ? "Player 2 has reached the maximum number of errors. Game over!\n"
                    : "����� 2 ������ ������������� ���������� ������. ���� ��������!\n") << RESET_COLOR;
                exit(0);
            }
        }
    }

    void startPlayerTimer(int playerTurn) {
        if (playerTurn == 1) {
            player1StartTime = steady_clock::now();
        }
        else {
            player2StartTime = steady_clock::now();
        }
    }

    void endPlayerTimer(int playerTurn) {
        auto endTime = steady_clock::now();
        auto duration = duration_cast<seconds>(endTime - (playerTurn == 1 ? player1StartTime : player2StartTime)).count();
        cout << YELLOW_TEXT << (currentLanguage == ENGLISH ? "Player " : "����� ")
            << playerTurn << (currentLanguage == ENGLISH ? " took " : " �������� ") << duration
            << (currentLanguage == ENGLISH ? " seconds.\n" : " ������.\n") << RESET_COLOR;
    }

    void saveGame(const string& filename) {
        ofstream file(filename);
        if (file.is_open()) {
            file << currentLanguage << " " << currentDifficulty << endl;
            for (int i = 0; i < GRID_SIZE; i++) {
                for (int j = 0; j < GRID_SIZE; j++) {
                    file << grid[i][j] << " ";
                }
                file << endl;
            }
            file.close();
            cout << GREEN_TEXT << (currentLanguage == ENGLISH ? "Game saved successfully.\n" : "���� ������� ���������.\n") << RESET_COLOR;
        }
        else {
            cout << RED_TEXT << (currentLanguage == ENGLISH ? "Error saving game.\n" : "������ ���������� ����.\n") << RESET_COLOR;
        }
    }

    void loadGame(const string& filename) {
        ifstream file(filename);
        if (file.is_open()) {
            int lang, diff;
            file >> lang >> diff;
            currentLanguage = static_cast<Language>(lang);
            currentDifficulty = static_cast<Difficulty>(diff);
            for (int i = 0; i < GRID_SIZE; i++) {
                for (int j = 0; j < GRID_SIZE; j++) {
                    file >> grid[i][j];
                }
            }
            file.close();
            cout << GREEN_TEXT << (currentLanguage == ENGLISH ? "Game loaded successfully.\n" : "���� ������� ���������.\n") << RESET_COLOR;
        }
        else {
            cout << RED_TEXT << (currentLanguage == ENGLISH ? "Error loading game.\n" : "������ �������� ����.\n") << RESET_COLOR;
        }
    }

    void pauseGame() {
        cout << (currentLanguage == ENGLISH ? "Game paused. Press any key to resume...\n" : "���� �� �����. ������� ����� �������, ����� ����������...\n");
        cin.ignore();
        cin.get();
    }

    void setTheme(Theme theme) {
        currentTheme = theme;
        switch (theme) {
        case DARK:
            cout << "\033[40;97m"; // ������ ���, ������� �����
            break;
        case LIGHT:
            cout << "\033[47;30m"; // ������� ���, ������ �����
            break;
        default:
            cout << RESET_COLOR; // ����� �� ����������� ������
            break;
        }
    }

    void changeTheme() {
        cout << (currentLanguage == ENGLISH ? "Choose theme: 1. Default 2. Dark 3. Light\n" : "�������� ����: 1. �� ��������� 2. Ҹ���� 3. �������\n");
        int themeChoice;
        cin >> themeChoice;
        if (themeChoice == 1) setTheme(DEFAULT);
        else if (themeChoice == 2) setTheme(DARK);
        else if (themeChoice == 3) setTheme(LIGHT);
    }

    void addPlayerToLeaderboard(const string& name, int score) {
        leaderboard.push_back({ name, score });
    }

    void displayLeaderboard() {
        cout << (currentLanguage == ENGLISH ? "\nLeaderboard:\n" : "\n��������� �������:\n");
        for (const auto& player : leaderboard) {
            cout << player.name << ": " << player.score << endl;
        }
    }

public:
    Sudoku() : currentLanguage(ENGLISH), currentDifficulty(EASY), currentTheme(DEFAULT), errorCount1(0), errorCount2(0) {
        enableANSIColors();
        generatePuzzle();
    }

    void setLanguage(Language lang) {
        currentLanguage = lang;
    }

    void setDifficulty(Difficulty diff) {
        currentDifficulty = diff;
    }

    void playGame() {
        startTime = steady_clock::now();
        showLocalTime();

        int row, col, num;
        string player1, player2;
        int playerTurn = 1;
        int score1 = 0, score2 = 0;

        cout << (currentLanguage == ENGLISH ? "Enter Player 1 name: " : "������� ��� ������ 1: ");
        cin >> player1;
        cout << (currentLanguage == ENGLISH ? "Enter Player 2 name: " : "������� ��� ������ 2: ");
        cin >> player2;

        while (true) {
            displayGrid();
            cout << (currentLanguage == ENGLISH ? "\nPlayer " : "\n����� ")
                << (playerTurn == 1 ? player1 : player2)
                << (currentLanguage == ENGLISH ? "'s turn.\n1. Make a Move\n2. Undo Move\n3. Hint\n4. Save Game\n5. Load Game\n6. Pause\n7. Exit\n8. Change Theme\nChoose an option: "
                    : " ���.\n1. ������� ���\n2. �������� ���\n3. ���������\n4. ��������� ����\n5. ��������� ����\n6. �����\n7. �����\n8. ������� ����\n�������� �����: ");
            int choice;
            cin >> choice;

            if (choice == 1) {
                cout << (currentLanguage == ENGLISH ? "Enter row, column, and number to place (1-9): "
                    : "������� ���, ������� � ����� ��� ���������� (1-9): ");
                cin >> row >> col >> num;

                row--;
                col--;

                if (row >= 0 && row < GRID_SIZE && col >= 0 && col < GRID_SIZE && num >= 1 && num <= 9) {
                    if (isValidMove(row, col, num)) {
                        grid[row][col] = num;
                        moves.push({ row, col, num });
                        if (playerTurn == 1) score1++;
                        else score2++;
                        endPlayerTimer(playerTurn); // ����������� ������ ������� ��� �������� ������
                        playerTurn = (playerTurn == 1 ? 2 : 1);
                        startPlayerTimer(playerTurn); // �������� ������ ������� ��� ���������� ������
                        cout << GREEN_TEXT << (currentLanguage == ENGLISH ? "Move accepted.\n" : "��� ������.\n") << RESET_COLOR;
                    }
                    else {
                        handleInvalidMove(playerTurn);
                        cout << RED_TEXT << (currentLanguage == ENGLISH ? "Invalid move!" : "������������ ���!") << RESET_COLOR;
                    }
                }
                else {
                    cout << RED_TEXT << (currentLanguage == ENGLISH ? "Invalid row, column, or number! Please enter values between 1 and 9.\n"
                        : "������������ ���, ������� ��� �����! ����������, ������� �������� �� 1 �� 9.\n") << RESET_COLOR;
                }
            }
            else if (choice == 2) {
                undoLastMove();
            }
            else if (choice == 3) {
                showHint();
            }
            else if (choice == 4) {
                saveGame("saved_game.txt");
            }
            else if (choice == 5) {
                loadGame("saved_game.txt");
            }
            else if (choice == 6) {
                pauseGame();
            }
            else if (choice == 7) {
                auto endTime = steady_clock::now();
                auto duration = duration_cast<seconds>(endTime - startTime).count();
                cout << GREEN_TEXT << (currentLanguage == ENGLISH ? "Game duration: " : "������������ ����: ") << duration << " seconds.\n" << RESET_COLOR;
                cout << (currentLanguage == ENGLISH ? "Exiting game...\n" : "����� �� ����...\n");
                break;
            }
            else if (choice == 8) {
                changeTheme();
            }
            else {
                cout << RED_TEXT << (currentLanguage == ENGLISH ? "Invalid option!\n" : "������������ �����!\n") << RESET_COLOR;
            }
        }

        addPlayerToLeaderboard(player1, score1);
        addPlayerToLeaderboard(player2, score2);
        displayLeaderboard();
    }

    void showInstructions() {
        cout << (currentLanguage == ENGLISH ? "\n--- Instructions ---\n" : "\n--- ���������� ---\n");
        cout << (currentLanguage == ENGLISH ?
            "1. The game starts by entering player names.\n"
            "2. Players take turns to fill in the Sudoku grid by entering a number between 1 and 9.\n"
            "3. Each move requires you to specify the row, column, and the number you want to place.\n"
            "4. The game will check if the move is valid. If the move is invalid, the player will be penalized.\n"
            "5. You can undo your last move or get hints during the game.\n"
            "6. You can pause the game, save, or load a saved game.\n"
            "7. The game ends when one of the players reaches the goal or if a player makes too many invalid moves.\n"
            "8. You can also choose a theme for the game.\n"
            :
        "1. ���� ���������� � ����� ���� �������.\n"
            "2. ������ ���������� ��������� ���� ������, ����� ����� �� 1 �� 9.\n"
            "3. ������ ��� ������� �������� ����, ������� � �����, ������� �� ������ ���������.\n"
            "4. ���� ��������, �������� �� ��� ����������. ���� ��� ������������, ����� ����� ����������.\n"
            "5. �� ������ �������� ��������� ��� ��� �������� ��������� � �������� ����.\n"
            "6. �� ������ ��������� ���� �� �����, ��������� ��� ��������� ����������� ����.\n"
            "7. ���� �������������, ����� ���� �� ������� ��������� ���� ��� ������ ������� ����� �������� �����.\n"
            "8. �� ����� ������ ������� ���� ��� ����.\n");
    }

    void displayPlayerStats() {
        vector<PlayerStats> players = loadStatsFromFile();  // ��������� ���������� �� �����

        if (players.empty()) {
            cout << (currentLanguage == ENGLISH ? "No saved player stats found." : "��� ����������� ���������� �������.") << endl;
            return;
        }

        // ������� ���������� ������� ������
        for (auto& player : players) {
            player.displayStats();  // ���������� displayStats(), ����� �������� ����������� ����������
            cout << endl;
        }
    }

    void mainMenu() {
        int choice;
        while (true) {
            cout << GREEN_TEXT << (currentLanguage == ENGLISH ? "\nSudoku Game\n" : "\n���� ������\n") << RESET_COLOR;
            cout << YELLOW_TEXT << (currentLanguage == ENGLISH ? "1. Start Game\n" : "1. ������ ����\n") << RESET_COLOR;
            cout << YELLOW_TEXT << (currentLanguage == ENGLISH ? "2. Daily Puzzle\n" : "2. ���������� �������\n") << RESET_COLOR;
            cout << YELLOW_TEXT << (currentLanguage == ENGLISH ? "3. View Leaderboard\n" : "3. �������� ��������� �������\n") << RESET_COLOR;
            cout << YELLOW_TEXT << (currentLanguage == ENGLISH ? "4. Change Language\n" : "4. ������� ����\n") << RESET_COLOR;
            cout << YELLOW_TEXT << (currentLanguage == ENGLISH ? "5. Instructions\n" : "5. ����������\n") << RESET_COLOR;
            cout << YELLOW_TEXT << (currentLanguage == ENGLISH ? "6. Set Difficulty\n" : "6. ���������� ���������\n") << RESET_COLOR;
            cout << YELLOW_TEXT << (currentLanguage == ENGLISH ? "7. Change Theme\n" : "7. ������� ����\n") << RESET_COLOR;
            cout << YELLOW_TEXT << (currentLanguage == ENGLISH ? "8. View Player Stats\n" : "9. �������� ���������� �������\n") << RESET_COLOR;
            cout << YELLOW_TEXT << (currentLanguage == ENGLISH ? "9. Exit\n" : "9. �����\n") << RESET_COLOR;
            cout << (currentLanguage == ENGLISH ? "Choose an option: " : "�������� �����: ");
            cin >> choice;

            if (choice == 1) {
                generatePuzzle();
                playGame();
            }
            else if (choice == 2) {
                generateDailyPuzzle();
                playGame();
            }
            else if (choice == 3) {
                displayLeaderboard();
            }
            else if (choice == 5) {
                showInstructions();
            }
            else if (choice == 4) {
                cout << (currentLanguage == ENGLISH ? "Choose language: 1. English 2. Russian\n"
                    : "�������� ����: 1. ���������� 2. �������\n");
                int langChoice;
                cin >> langChoice;
                if (langChoice == 1) setLanguage(ENGLISH);
                else if (langChoice == 2) setLanguage(RUSSIAN);
            }
            else if (choice == 6) {
                cout << (currentLanguage == ENGLISH ? "Select Difficulty: 1. Easy 2. Medium 3. Hard\n"
                    : "�������� ���������: 1. ����� 2. ������ 3. ������\n");
                int diffChoice;
                cin >> diffChoice;
                if (diffChoice == 1) setDifficulty(EASY);
                else if (diffChoice == 2) setDifficulty(MEDIUM);
                else if (diffChoice == 3) setDifficulty(HARD);
            }
            else if (choice == 7) {
                changeTheme();
            }
            else if (choice == 8) {
                displayPlayerStats();
            }
            else if (choice == 9) {
                cout << (currentLanguage == ENGLISH ? "Goodbye!\n" : "�� ��������!\n");
                break;
            }
            else {
                cout << RED_TEXT << (currentLanguage == ENGLISH ? "Invalid option!\n" : "������������ �����!\n") << RESET_COLOR;
            }
        }
    }
};


int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    setlocale(LC_ALL, "Russian");
    Sudoku game;
    game.mainMenu();
    vector<PlayerStats> players = loadStatsFromFile();
    PlayerStats player1 = players.size() > 0 ? players[0] : PlayerStats("Player 1");
    PlayerStats player2 = players.size() > 1 ? players[1] : PlayerStats("Player 2");

    cout << "Game Start!" << endl;

    auto start = chrono::high_resolution_clock::now();

    // ������ ����

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    bool player1Win = true;
    displayGameResults(player1, player2, duration.count());
    return 0;
}