#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <string>
#include <algorithm>
#include <mutex>
#include <condition_variable>
#include <limits>

using namespace std;

// ANSI escape codes for colors
const string RESET = "\033[0m";
const string RED = "\033[31m";
const string GREEN = "\033[32m";
const string YELLOW = "\033[33m";
const string BLUE = "\033[34m";
const string MAGENTA = "\033[35m";
const string CYAN = "\033[36m";
const string WHITE = "\033[37m";

#ifdef _WIN32
void clearScreen() {
    system("cls");
}
#else
void clearScreen() {
    system("clear");
}
#endif

const int WINDOW_WIDTH = 1400;
const int WINDOW_HEIGHT = 230;
const int BAR_GAP = 5;
const int MAX_VISUALIZATIONS = 3;

SDL_Window* windows[6] = {nullptr};
SDL_Renderer* renderers[6] = {nullptr};
TTF_Font* font = nullptr;

bool quit = false;
bool paused = false;
int delay = 100;

std::mutex mtx;
std::condition_variable cv;
std::mutex render_mtx;

bool init(int options[], int count) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    if (TTF_Init() < 0) {
        std::cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
        return false;
    }

    font = TTF_OpenFont("arial.ttf", 13);
    if (!font) {
        std::cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << std::endl;
        return false;
    }

    string titles[] = {"Selection Sort Visualizer", "Insertion Sort Visualizer", "Bubble Sort Visualizer", "Merge Sort Visualizer", "Quick Sort Visualizer", "Heap Sort Visualizer"};

    for (int k = 0; k < count; ++k) {
        int opt = options[k];
        if (opt < 1 || opt > 6) {
            std::cerr << "Invalid option!" << std::endl;
            return false;
        }

        int i = opt - 1;
        int posX = 20;
        int posY = 40 + (i % 3) * (WINDOW_HEIGHT + 30);

        windows[i] = SDL_CreateWindow(titles[i].c_str(), posX, posY, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
        if (!windows[i]) {
            std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            SDL_Quit();
            return false;
        }
        renderers[i] = SDL_CreateRenderer(windows[i], -1, SDL_RENDERER_ACCELERATED);
        if (!renderers[i]) {
            std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            SDL_DestroyWindow(windows[i]);
            SDL_Quit();
            return false;
        }
    }
    return true;
}

void close() {
    for (int i = 0; i < 6; ++i) {
        if (renderers[i]) SDL_DestroyRenderer(renderers[i]);
        if (windows[i]) SDL_DestroyWindow(windows[i]);
    }
    TTF_CloseFont(font);
    font = nullptr;
    TTF_Quit();
    SDL_Quit();
}

void renderText(SDL_Renderer* renderer, const std::string& text, int x, int y) {
    SDL_Color color = {255, 255, 255};  // White color for text
    SDL_Surface* surfaceMessage = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!surfaceMessage) {
        std::cerr << "Failed to create surface for text! TTF_Error: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Texture* message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
    if (!message) {
        std::cerr << "Failed to create texture from surface! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(surfaceMessage);
        return;
    }

    SDL_Rect messageRect = {x, y, surfaceMessage->w, surfaceMessage->h};

    SDL_RenderCopy(renderer, message, nullptr, &messageRect);
    SDL_FreeSurface(surfaceMessage);
    SDL_DestroyTexture(message);
}

void renderSort(SDL_Renderer* renderer, const std::vector<int>& arr, size_t currentIndex, size_t secondIndex, const std::string& mode) {
    std::lock_guard<std::mutex> lock(render_mtx);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer);

    int numElements = arr.size();
    int barWidth = (WINDOW_WIDTH - (BAR_GAP * (numElements - 1))) / numElements;

    for (size_t i = 0; i < arr.size(); ++i) {
        int height = (arr[i] * (WINDOW_HEIGHT - 40)) / 100;
        SDL_Rect bar = {static_cast<int>(i * (barWidth + BAR_GAP)), WINDOW_HEIGHT - height - 30, barWidth, height};

        if (mode == "update") {
            SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
        } else if (mode == "selection") {
            if (i == currentIndex) {
                SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
            } else if (i == secondIndex) {
                SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 0xFF);
            } else {
                SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
            }
        } else if (mode == "insertion") {
            if (i < currentIndex) {
                SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
            } else if (i == secondIndex) {
                SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 0xFF);
            } else {
                SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
            }
        } else {
            if (i == currentIndex) {
                SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
            } else if (i == secondIndex) {
                SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 0xFF);
            } else {
                SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
            }
        }

        SDL_RenderFillRect(renderer, &bar);

        int textYPos = std::max(WINDOW_HEIGHT - height - 30 - 20, 0);
        renderText(renderer, std::to_string(arr[i]), static_cast<int>(i * (barWidth + BAR_GAP)), textYPos);
    }

    SDL_RenderPresent(renderer);
}

void handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            std::lock_guard<std::mutex> lock(mtx);
            quit = true;
            cv.notify_all();
        }
        else if (e.type == SDL_KEYDOWN) {
            std::lock_guard<std::mutex> lock(mtx);
            if (e.key.keysym.sym == SDLK_ESCAPE) {
                quit = true;
                cv.notify_all();
            }
            else if (e.key.keysym.sym == SDLK_p) {
                paused = !paused;
                if (paused) {
                    clearScreen();
                    std::cout << RED << "Paused. Press 'P' to resume." << RESET << "\n";
                } else {
                    cv.notify_all();
                }
            }
            else if (e.key.keysym.sym == SDLK_RIGHT) {
                if (delay > 10) {
                    clearScreen();
                    delay -= 10;
                    std::cout << GREEN << "Speed increased. Delay: " << delay << "ms" << RESET << "\n";
                }
            }
            else if (e.key.keysym.sym == SDLK_LEFT) {
                clearScreen();
                delay += 10;
                std::cout << YELLOW << "Speed decreased. Delay: " << delay << "ms" << RESET << "\n";
            }
            else if (e.key.keysym.sym == SDLK_0) {
                std::vector<int> newArr(80);
                for (int i = 0; i < 80; ++i) {
                    newArr[i] = std::rand() % 100;
                }
                for (int i = 0; i < 6; ++i) {
                    renderSort(renderers[i], newArr, 0, 0, "update");
                }
                clearScreen();
                std::cout << BLUE << "New array generated and visualized." << RESET << "\n";
            }
        }
    }
}

void waitForResume() {
    std::unique_lock<std::mutex> lock(mtx);
    while (paused && !quit) {
        cv.wait(lock);
        handleEvents(); 
    }
    if (quit) {
        lock.unlock();
    }
}

void selectionSort(std::vector<int>& arr, SDL_Renderer* renderer) {
    for (size_t i = 0; i < arr.size() - 1; ++i) {
        size_t minIndex = i;
        for (size_t j = i + 1; j < arr.size(); ++j) {
            handleEvents();
            {
                std::lock_guard<std::mutex> lock(mtx);
                if (quit) return;
            }
            waitForResume();
            if (arr[j] < arr[minIndex]) {
                minIndex = j;
            }
            renderSort(renderer, arr, j, minIndex, "selection");
            SDL_Delay(delay);
        }
        std::swap(arr[i], arr[minIndex]);
        renderSort(renderer, arr, i, minIndex, "selection");
        SDL_Delay(delay);
    }
}

void insertionSort(std::vector<int>& arr, SDL_Renderer* renderer) {
    for (size_t i = 1; i < arr.size(); ++i) {
        int key = arr[i];
        size_t j = i - 1;

        while (j < arr.size() && arr[j] > key) {
            handleEvents();
            {
                std::lock_guard<std::mutex> lock(mtx);
                if (quit) return;
            }
            waitForResume();
            arr[j + 1] = arr[j];
            renderSort(renderer, arr, i, j + 1, "insertion");
            j--;
            SDL_Delay(delay);
        }
        arr[j + 1] = key;
        renderSort(renderer, arr, i, j + 1, "insertion");
        SDL_Delay(delay);
    }
}

void bubbleSort(std::vector<int>& arr, SDL_Renderer* renderer) {
    for (size_t i = 0; i < arr.size() - 1; ++i) {
        for (size_t j = 0; j < arr.size() - i - 1; ++j) {
            handleEvents();
            {
                std::lock_guard<std::mutex> lock(mtx);
                if (quit) return;  
            }
            waitForResume(); 
            if (arr[j] > arr[j + 1]) {
                std::swap(arr[j], arr[j + 1]);
            }
            renderSort(renderer, arr, j, j + 1, "bubble");
            SDL_Delay(delay);
        }
    }
}

void merge(std::vector<int>& arr, int left, int mid, int right, SDL_Renderer* renderer) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    std::vector<int> L(n1), R(n2);
    for (int i = 0; i < n1; ++i) L[i] = arr[left + i];
    for (int j = 0; j < n2; ++j) R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        handleEvents();
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (quit) return;  
        }
        waitForResume(); 
        if (L[i] <= R[j]) {
            arr[k++] = L[i++];
        } else {
            arr[k++] = R[j++];
        }
        renderSort(renderer, arr, k - 1, -1, "merge");
        SDL_Delay(delay);
    }

    while (i < n1) {
        handleEvents();
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (quit) return;  
        }
        waitForResume(); 
        arr[k++] = L[i++];
        renderSort(renderer, arr, k - 1, -1, "merge");
        SDL_Delay(delay);
    }
    while (j < n2) {
        handleEvents();
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (quit) return;  
        }
        waitForResume(); 
        arr[k++] = R[j++];
        renderSort(renderer, arr, k - 1, -1, "merge");
        SDL_Delay(delay);
    }
}

void mergeSort(std::vector<int>& arr, int left, int right, SDL_Renderer* renderer) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid, renderer);
        mergeSort(arr, mid + 1, right, renderer);
        merge(arr, left, mid, right, renderer);
    }
}

void quickSort(std::vector<int>& arr, int low, int high, SDL_Renderer* renderer) {
    if (low < high) {
        int pivot = arr[high];
        int i = low - 1;
        for (int j = low; j <= high - 1; ++j) {
            handleEvents();
            {
                std::lock_guard<std::mutex> lock(mtx);
                if (quit) return;  
            }
            waitForResume(); 
            if (arr[j] < pivot) {
                ++i;
                std::swap(arr[i], arr[j]);
            }
            renderSort(renderer, arr, j, high, "quick");
            SDL_Delay(delay);
        }
        std::swap(arr[i + 1], arr[high]);
        renderSort(renderer, arr, i + 1, high, "quick");
        SDL_Delay(delay);
        
        quickSort(arr, low, i, renderer);
        quickSort(arr, i + 2, high, renderer);
    }
}

void heapify(std::vector<int>& arr, int n, int i, SDL_Renderer* renderer) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n && arr[left] > arr[largest]) largest = left;
    if (right < n && arr[right] > arr[largest]) largest = right;

    if (largest != i) {
        std::swap(arr[i], arr[largest]);
        renderSort(renderer, arr, largest, i, "heap");
        SDL_Delay(delay);
        handleEvents();
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (quit) return;  
        }
        waitForResume(); 
        heapify(arr, n, largest, renderer);
    }
}

void heapSort(std::vector<int>& arr, SDL_Renderer* renderer) {
    int n = arr.size();
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i, renderer);
        renderSort(renderer, arr, i, -1, "heap");
        SDL_Delay(delay);
    }
    for (int i = n - 1; i > 0; i--) {
        std::swap(arr[0], arr[i]);
        renderSort(renderer, arr, i, -1, "heap");
        SDL_Delay(delay);
        heapify(arr, i, 0, renderer);
    }
}

void executeSorting(int option) {
    std::vector<int> arr(70);
    for (int i = 0; i < 70; ++i) {
        arr[i] = std::rand() % 100;
    }

    switch (option) {
        case 1:
            selectionSort(arr, renderers[0]);
            break;
        case 2:
            insertionSort(arr, renderers[1]);
            break;
        case 3:
            bubbleSort(arr, renderers[2]);
            break;
        case 4:
            mergeSort(arr, 0, arr.size() - 1, renderers[3]);
            break;
        case 5:
            quickSort(arr, 0, arr.size() - 1, renderers[4]);
            break;
        case 6:
            heapSort(arr, renderers[5]);
            break;
        default:
            std::cerr << "Invalid option!" << std::endl;
            break;
    }
}

void showWelcomeMessage() {
    clearScreen();
 
   
    cout << CYAN << "==============================" << RESET << endl;
    cout << YELLOW << "  Welcome to the Sorting Visualizer!" << RESET << endl;
    cout << CYAN << "==============================" << RESET << endl;
    cout << GREEN << "This sorting visualizer visualizes multiple sorting algorithms:" << RESET << endl;
    cout << GREEN << " - Insertion Sort" << RESET << endl;
    cout << GREEN << " - Selection Sort" << RESET << endl;
    cout << GREEN << " - Merge Sort" << RESET << endl;
    cout << GREEN << " - Bubble Sort" << RESET << endl;
    cout << GREEN << " - Quick Sort" << RESET << endl;
    cout << GREEN << " - Heap Sort" << RESET << endl;
    cout << BLUE << "\nYou can speed up or slow down using the left and right arrow keys, respectively." << RESET << endl;
    cout << BLUE << "Press 'P' to pause and 'ESC' to quit the window." << RESET << endl;
    cout << RED << "\nCaution: Pressing multiple keys during multiple visualizations may cause the program to crash!" << RESET << endl;
    cout << CYAN << "\nPress 'Y' to continue to the main menu..." << RESET << endl;



    char response;
    std::cin >> response;
    while (response != 'Y' && response != 'y') {
        std::cout << RED << "Invalid input! Please press 'Y' to continue." << RESET << std::endl;
        std::cin >> response;
    }
}

void showmenu(){
        cout << CYAN << "=====================" << RESET << endl;
        cout << YELLOW << "  Main Menu\n" << RESET;
        cout << CYAN << "=====================" << RESET << endl;
        cout << GREEN << "1. One Visualization\n" << RESET;
        cout << GREEN << "2. Multiple Visualizations\n" << RESET;
        cout << GREEN << "3. Change Speed\n" << RESET;
        cout << GREEN << "4. Exit\n" << RESET;
        cout << BLUE << "Enter your choice: " << RESET;
}
void showSingleVisualizationMenu() {
    clearScreen();
    int sortOption;
    std::cout << "Select sorting algorithm to visualize:\n";
    std::cout << GREEN << "1. Selection Sort\n" << RESET;
    std::cout << GREEN << "2. Insertion Sort\n" << RESET;
    std::cout << GREEN << "3. Bubble Sort\n" << RESET;
    std::cout << GREEN << "4. Merge Sort\n" << RESET;
    std::cout << GREEN << "5. Quick Sort\n" << RESET;
    std::cout << GREEN << "6. Heap Sort\n" << RESET;
    std::cout << BLUE << "Enter your choice (1-6): " << RESET;
    std::cin >> sortOption;

    if (std::cin.fail() || sortOption < 1 || sortOption > 6) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cerr << RED << "Invalid choice! Please enter a number between 1 and 6." << RESET << "\n";
        return;
    }

    int options[] = {sortOption};
    if (!init(options, 1)) {
        return;
    }

    std::thread sortingThread(executeSorting, sortOption);
    while (!quit) {
        handleEvents();
        SDL_Delay(10);
    }
    sortingThread.join();
    close();
    quit = false;
}

void showMultipleVisualizationsMenu() {
    clearScreen();
    int numSorts;
    std::cout << "How many sorting algorithms to visualize (1-" << MAX_VISUALIZATIONS << "): ";
    std::cin >> numSorts;

    if (std::cin.fail() || numSorts < 1 || numSorts > MAX_VISUALIZATIONS) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cerr << RED << "Invalid number of visualizations! Please enter a number between 1 and " << MAX_VISUALIZATIONS << "." << RESET << "\n";
        return;
    }
    clearScreen();

    int options[6];
    std::cout << "Select the sorting algorithms to visualize:\n";
    std::cout << GREEN << "1. Selection Sort\n" << RESET;
    std::cout << GREEN << "2. Insertion Sort\n" << RESET;
    std::cout << GREEN << "3. Bubble Sort\n" << RESET;
    std::cout << GREEN << "4. Merge Sort\n" << RESET;
    std::cout << GREEN << "5. Quick Sort\n" << RESET;
    std::cout << GREEN << "6. Heap Sort\n" << RESET;
    for (int i = 0; i < numSorts; ++i) {
        std::cout << BLUE << "Enter choice " << (i + 1) << " (1-6): " << RESET;
        std::cin >> options[i];

        if (std::cin.fail() || options[i] < 1 || options[i] > 6) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cerr << RED << "Invalid choice! Please enter a number between 1 and 6." << RESET << "\n";
            --i; 
        }
    }

    if (!init(options, numSorts)) {
        return;
    }

    std::vector<std::thread> threads;
    for (int i = 0; i < numSorts; ++i) {
        threads.push_back(std::thread(executeSorting, options[i]));
    }

    while (!quit) {
        handleEvents();
        SDL_Delay(10);
    }

    for (auto& thread : threads) {
        if (thread.joinable()) thread.join();
    }
    close();
    quit = false;
}

void changeSpeed() {
    clearScreen();
    int speedOption;
    std::cout << CYAN << "Select speed:\n" << RESET;
    std::cout << GREEN << "1. Slow\n" << RESET;
    std::cout << GREEN << "2. Medium\n" << RESET;
    std::cout << GREEN << "3. Fast\n" << RESET;
    std::cout << BLUE << "Enter your choice: " << RESET;
    std::cin >> speedOption;

    switch (speedOption) {
        case 1:
            delay = 300;
            break;
        case 2:
            delay = 100;
            break;
        case 3:
            delay = 50;
            break;
        default:
            std::cerr << RED << "Invalid choice! Using default speed (Medium).\n" << RESET;
            delay = 100;
            break;
    }
}



int main() {
    srand(static_cast<unsigned int>(time(0)));

    showWelcomeMessage();
    int menuChoice;
    bool running = true;

    while (running) {
        clearScreen();
        showmenu();
        std::cin >> menuChoice;

        if (std::cin.fail() || menuChoice < 1 || menuChoice > 4) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cerr << RED << "Invalid choice! Please enter a number between 1 and 4." << RESET << "\n";
            continue;
        }

        switch (menuChoice) {
            case 1:
                showSingleVisualizationMenu();
                break;
            case 2:
                showMultipleVisualizationsMenu();
                break;
            case 3:
                changeSpeed();
                break;
            case 4:
                running = false;
                break;
            default:
                std::cerr << RED << "Invalid choice! Please enter a number between 1 and 4." << RESET << "\n";
                break;
        }
    }

    close();
    return 0;
}
