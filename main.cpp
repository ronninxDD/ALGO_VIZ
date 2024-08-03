#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
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

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 300;
const int BAR_WIDTH = 10;

SDL_Window* windows[6] = {nullptr};
SDL_Renderer* renderers[6] = {nullptr};

bool quit = false;
bool paused = false; // New variable to control pause state
int delay = 100; // Default delay for speed

std::mutex mtx;
std::condition_variable cv;

bool init(int options[], int count) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
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
        windows[i] = SDL_CreateWindow(titles[i].c_str(), SDL_WINDOWPOS_CENTERED + (i % 2) * WINDOW_WIDTH, SDL_WINDOWPOS_CENTERED + (i / 2) * WINDOW_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
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
    SDL_Quit();
}

void handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            std::lock_guard<std::mutex> lock(mtx);
            quit = true;
            cv.notify_all(); // Notify all threads to exit
        }
        else if (e.type == SDL_KEYDOWN) {
            std::lock_guard<std::mutex> lock(mtx);
            if (e.key.keysym.sym == SDLK_ESCAPE) {
                quit = true;
                cv.notify_all(); // Notify all threads to exit
            }
            else if (e.key.keysym.sym == SDLK_p) { // Pause/Resume with 'P' key
                paused = !paused;
                if (paused) {
                    std::cout << "Paused. Press 'P' to resume.\n";
                } else {
                    cv.notify_all(); // Resume all threads
                }
            }
            else if (e.key.keysym.sym == SDLK_RIGHT) { // Speed up with 'Right Arrow'
                if (delay > 10) {
                    delay -= 10;
                    std::cout << "Speed increased. Delay: " << delay << "ms\n";
                }
            }
            else if (e.key.keysym.sym == SDLK_LEFT) { // Slow down with 'Left arrow'
                delay += 10;
                std::cout << "Speed decreased. Delay: " << delay << "ms\n";
            }
        }
    }
}

void renderSort(SDL_Renderer* renderer, const std::vector<int>& arr, size_t currentIndex, size_t secondIndex, const std::string& mode) {
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF); // Black color for bars
    SDL_RenderClear(renderer);
    
    for (size_t i = 0; i < arr.size(); ++i) {
        int height = (arr[i] * (WINDOW_HEIGHT - 20)) / 100; // Scale height
        SDL_Rect bar = {static_cast<int>(i * BAR_WIDTH), WINDOW_HEIGHT - height - 20, BAR_WIDTH - 1, height};
        
        if (mode == "selection") {
            if (i == currentIndex) {
                SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF); // Red for current index
            } else if (i == secondIndex) {
                SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 0xFF); // Blue for min index
            } else {
                SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF); // Green for sorted parts
            }
        } else if (mode == "insertion") {
            if (i < currentIndex) {
                SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF); // Green for sorted parts
            } else if (i == secondIndex) {
                SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 0xFF); // Blue for index being inserted
            } else {
                SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF); // Red for unsorted parts
            }
        } else {
            if (i == currentIndex) {
                SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF); // Red for the current index
            } else if (i == secondIndex) {
                SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 0xFF); // Blue for the second index
            } else {
                SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF); // Green for sorted parts
            }
        }
        
        SDL_RenderFillRect(renderer, &bar);
    }
    
    SDL_RenderPresent(renderer);
}

void waitForResume() {
    std::unique_lock<std::mutex> lock(mtx);
    while (paused && !quit) {
        cv.wait(lock);
        handleEvents(); // Check for quit event while paused
    }
    // Check for quit after resuming
    if (quit) {
        lock.unlock(); // Unlock before exiting the function
    }
}


void selectionSort(std::vector<int>& arr, SDL_Renderer* renderer) {
    for (size_t i = 0; i < arr.size() - 1; ++i) {
        size_t minIndex = i;
        for (size_t j = i + 1; j < arr.size(); ++j) {
            handleEvents();
            {
                std::lock_guard<std::mutex> lock(mtx);
                if (quit) return;  // Exit if quit is flagged
            }
            waitForResume(); // Wait if paused
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

        while (j >= 0 && arr[j] > key) { 
            handleEvents();
            {
                std::lock_guard<std::mutex> lock(mtx);
                if (quit) return;  // Exit if quit is flagged
            }
            waitForResume(); // Wait if paused
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
                if (quit) return;  // Exit if quit is flagged
            }
            waitForResume(); // Wait if paused
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
            if (quit) return;  // Exit if quit is flagged
        }
        waitForResume(); // Wait if paused
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
            if (quit) return;  // Exit if quit is flagged
        }
        waitForResume(); // Wait if paused
        arr[k++] = L[i++];
        renderSort(renderer, arr, k - 1, -1, "merge");
        SDL_Delay(delay);
    }
    while (j < n2) {
        handleEvents();
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (quit) return;  // Exit if quit is flagged
        }
        waitForResume(); // Wait if paused
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
        int pivot = arr[high]; // choose rightmost element as pivot
        int i = low - 1;
        for (int j = low; j <= high - 1; ++j) {
            handleEvents();
            {
                std::lock_guard<std::mutex> lock(mtx);
                if (quit) return;  // Exit if quit is flagged
            }
            waitForResume(); // Wait if paused
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
        renderSort(renderer, arr, largest, i, "heap"); // Visualize swap
        SDL_Delay(delay); // Delay to visualize the change
        handleEvents();
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (quit) return;  // Exit if quit is flagged
        }
        waitForResume(); // Wait if paused
        heapify(arr, n, largest, renderer);
    }
}

void heapSort(std::vector<int>& arr, SDL_Renderer* renderer) {
    int n = arr.size();
    // Build heap (rearrange array)
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i, renderer);
        renderSort(renderer, arr, i, -1, "heap"); // Visualize heap formation
        SDL_Delay(delay); // Delay to visualize the heap building
    }
    // One by one extract an element from heap
    for (int i = n - 1; i > 0; i--) {
        std::swap(arr[0], arr[i]);
        renderSort(renderer, arr, i, -1, "heap"); // Visualize the array after swap
        SDL_Delay(delay); // Delay to visualize the change
        heapify(arr, i, 0, renderer);
    }
}

void executeSorting(int option) {
    std::vector<int> arr(80);
    for (int i = 0; i < 80; ++i) {
        arr[i] = std::rand() % 100; // Random values between 0 and 99
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

    {
        std::lock_guard<std::mutex> lock(mtx);
        quit = true; // Set quit flag to true after sorting is done
    }
    cv.notify_all(); // Notify all threads to exit
}

void showMenu() {
    std::cout << "Main Menu:\n";
    std::cout << "1. One Visualization\n";
    std::cout << "2. Multiple Visualizations\n";
    std::cout << "3. Change Speed\n";
    std::cout << "4. Exit\n";
    std::cout << "Enter your choice: ";
}

void changeSpeed() {
    int speedOption;
    std::cout << "Select speed:\n";
    std::cout << "1. Slow\n";
    std::cout << "2. Medium\n";
    std::cout << "3. Fast\n";
    std::cout << "Enter your choice: ";
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
            std::cerr << "Invalid choice! Using default speed (Medium).\n";
            delay = 100;
            break;
    }
}

int main() {
    srand(static_cast<unsigned int>(time(0)));

    int menuChoice;
    bool running = true;

    while (running) {
        showMenu();
        std::cin >> menuChoice;

        if (std::cin.fail() || menuChoice < 1 || menuChoice > 4) {
            std::cin.clear(); // Clear the error flag
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore invalid input
            std::cerr << "Invalid choice! Please enter a number between 1 and 4.\n";
            continue;
        }

        switch (menuChoice) {
            case 1: {
                int sortOption;
                std::cout << "Select sorting algorithm to visualize:\n";
                std::cout << "1. Selection Sort\n";
                std::cout << "2. Insertion Sort\n";
                std::cout << "3. Bubble Sort\n";
                std::cout << "4. Merge Sort\n";
                std::cout << "5. Quick Sort\n";
                std::cout << "6. Heap Sort\n";
                std::cout << "Enter your choice (1-6): ";
                std::cin >> sortOption;

                if (std::cin.fail() || sortOption < 1 || sortOption > 6) {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cerr << "Invalid choice! Please enter a number between 1 and 6.\n";
                    break;
                }

                int options[] = {sortOption};
                if (!init(options, 1)) {
                    return 1;
                }

                std::thread sortingThread(executeSorting, sortOption);
                while (!quit) {
                    handleEvents();
                    SDL_Delay(10); // Small delay to prevent busy-waiting
                }
                if (sortingThread.joinable()) sortingThread.join();
                close();
                quit = false;
                break;
            }
            case 2: {
                int numSorts;
                std::cout << "How many sorting algorithms to visualize (1-6): ";
                std::cin >> numSorts;

                if (std::cin.fail() || numSorts < 1 || numSorts > 6) {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cerr << "Invalid number of visualizations! Please enter a number between 1 and 6.\n";
                    break;
                }

                int options[6];
                std::cout << "Select the sorting algorithms to visualize:\n";
                std::cout << "1. Selection Sort\n";
                std::cout << "2. Insertion Sort\n";
                std::cout << "3. Bubble Sort\n";
                std::cout << "4. Merge Sort\n";
                std::cout << "5. Quick Sort\n";
                std::cout << "6. Heap Sort\n";
                for (int i = 0; i < numSorts; ++i) {
                    std::cout << "Enter choice " << (i + 1) << " (1-6): ";
                    std::cin >> options[i];

                    if (std::cin.fail() || options[i] < 1 || options[i] > 6) {
                        std::cin.clear();
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::cerr << "Invalid choice! Please enter a number between 1 and 6.\n";
                        --i; // Retry the current input
                    }
                }

                if (!init(options, numSorts)) {
                    return 1;
                }

                std::vector<std::thread> threads;
                for (int i = 0; i < numSorts; ++i) {
                    threads.push_back(std::thread(executeSorting, options[i]));
                }

                while (!quit) {
                    handleEvents();
                    SDL_Delay(10); // Small delay to prevent busy-waiting
                }

                for (auto& thread : threads) {
                    if (thread.joinable()) thread.join();
                }
                close();
                quit = false;
                break;
            }
            case 3:
                changeSpeed();
                break;
            case 4:
                running = false;
                break;
            default:
                std::cerr << "Invalid choice! Please enter a number between 1 and 4.\n";
                break;
        }
    }

    return 0;
}
