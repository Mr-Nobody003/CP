#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <cstdlib>
#include <fcntl.h>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif

class Matrix {
private:
    int width, height;
    std::vector<std::vector<char>> screen;
    std::vector<std::vector<int>> brightness; // For fading effect
    std::vector<int> drops;
    std::vector<int> speeds;
    std::vector<int> lengths;
    std::vector<int> counters; // For speed control
    std::mt19937 rng;
    std::uniform_int_distribution<int> char_dist;
    std::uniform_int_distribution<int> speed_dist;
    std::uniform_int_distribution<int> length_dist;
    std::uniform_int_distribution<int> spawn_dist;
    
    // Matrix characters (mix of katakana, numbers, and symbols)
    const std::string matrix_chars = "アイウエオカキクケコサシスセソタチツテトナニヌネノハヒフヘホマミムメモヤユヨラリルレロワヲン0123456789!@#$%^&*()";

public:
    Matrix() : rng(std::chrono::steady_clock::now().time_since_epoch().count()) {
        getTerminalSize();
        initializeMatrix();
    }
    
    void getTerminalSize() {
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        width = w.ws_col;
        height = w.ws_row;
#endif
    }
    
    void initializeMatrix() {
        screen.resize(height, std::vector<char>(width, ' '));
        brightness.resize(height, std::vector<int>(width, 0));
        drops.resize(width);
        speeds.resize(width);
        lengths.resize(width);
        counters.resize(width);
        
        char_dist = std::uniform_int_distribution<int>(0, matrix_chars.length() - 1);
        speed_dist = std::uniform_int_distribution<int>(1, 4); // Faster speeds
        length_dist = std::uniform_int_distribution<int>(7, 10); // Shorter trails
        spawn_dist = std::uniform_int_distribution<int>(0, 50); // More frequent spawning
        
        // Initialize drops - start them above screen
        for (int i = 0; i < width; ++i) {
            drops[i] = -spawn_dist(rng) - length_dist(rng); // Start above screen
            speeds[i] = speed_dist(rng);
            lengths[i] = length_dist(rng);
            counters[i] = 0;
        }
    }
    
    void clearScreen() {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }
    
    void hideCursor() {
#ifdef _WIN32
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
#else
        std::cout << "\033[?25l";
#endif
    }
    
    void showCursor() {
#ifdef _WIN32
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
        cursorInfo.bVisible = true;
        SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
#else
        std::cout << "\033[?25h";
#endif
    }
    
    void setGreenText(int brightness_level) {
#ifdef _WIN32
        if (brightness_level > 5) {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        } else {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
        }
#else
        if (brightness_level > 5) {
            std::cout << "\033[92m"; // Bright green
        } else {
            std::cout << "\033[32m"; // Dim green
        }
#endif
    }
    
    void resetColor() {
#ifdef _WIN32
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else
        std::cout << "\033[0m"; // Reset to default
#endif
    }
    
    bool kbhit() {
#ifdef _WIN32
        return _kbhit();
#else
        struct termios oldt, newt;
        int ch;
        int oldf;
        
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
        
        ch = getchar();
        
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);
        
        if (ch != EOF) {
            ungetc(ch, stdin);
            return true;
        }
        return false;
#endif
    }
    
    void update() {
        // Fade all characters faster
        for (int row = 0; row < height; ++row) {
            for (int col = 0; col < width; ++col) {
                if (brightness[row][col] > 0) {
                    brightness[row][col] -= 3; // Faster fading
                    if (brightness[row][col] <= 0) {
                        brightness[row][col] = 0;
                        screen[row][col] = ' ';
                    }
                }
            }
        }
        
        // Update each column
        for (int col = 0; col < width; ++col) {
            counters[col]++;
            
            // Only move drop when counter reaches speed threshold
            if (counters[col] >= speeds[col]) {
                counters[col] = 0;
                drops[col]++;
                
                // Draw the head of the drop (brightest)
                if (drops[col] >= 0 && drops[col] < height) {
                    screen[drops[col]][col] = matrix_chars[char_dist(rng)];
                    brightness[drops[col]][col] = 10; // Brightest
                }
                
                // Draw the tail with fading brightness
                for (int i = 1; i < lengths[col]; ++i) {
                    int tail_row = drops[col] - i;
                    if (tail_row >= 0 && tail_row < height) {
                        if (brightness[tail_row][col] < (10 - i)) {
                            screen[tail_row][col] = matrix_chars[char_dist(rng)];
                            brightness[tail_row][col] = std::max(1, 10 - i);
                        }
                    }
                }
                
                // Reset drop if it completely goes off screen
                if (drops[col] - lengths[col] > height) {
                    drops[col] = -spawn_dist(rng) - lengths[col];
                    speeds[col] = speed_dist(rng);
                    lengths[col] = length_dist(rng);
                }
            }
        }
    }
    
    void render() {
        // Move cursor to top-left
#ifdef _WIN32
        COORD coord = {0, 0};
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
#else
        std::cout << "\033[H";
#endif
        
        // Render screen with brightness levels
        for (int row = 0; row < height; ++row) {
            for (int col = 0; col < width; ++col) {
                if (screen[row][col] != ' ' && brightness[row][col] > 0) {
                    setGreenText(brightness[row][col]);
                    std::cout << screen[row][col];
                    resetColor();
                } else {
                    std::cout << ' ';
                }
            }
            if (row < height - 1) std::cout << '\n';
        }
        std::cout << std::flush;
    }
    
    void sleep_ms(int milliseconds) {
#ifdef _WIN32
        Sleep(milliseconds);
#else
        usleep(milliseconds * 1000);
#endif
    }
    
    void run() {
        clearScreen();
        hideCursor();
        
        std::cout << "Matrix Digital Rain - Press any key to exit\n";
        sleep_ms(2000);
        
        while (!kbhit()) {
            update();
            render();
            sleep_ms(8); // Even faster animation
        }
        
        showCursor();
        resetColor();
        clearScreen();
        std::cout << "Matrix effect terminated.\n";
    }
};

int main() {
    try {
        Matrix matrix;
        matrix.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
