#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <string>
#include <algorithm>

using namespace std;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 300;
const int BAR_WIDTH = 10;

SDL_Window* windows[6] = {nullptr};
SDL_Renderer* renderers[6] = {nullptr};

bool quit = false;

bool init(int firstOption, int secondOption) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    string titles[] = {"Selection Sort Visualizer", "Insertion Sort Visualizer", "Bubble Sort Visualizer", "Merge Sort Visualizer", "Quick Sort Visualizer", "Heap Sort Visualizer"};
    int options[] = {firstOption, secondOption};

    for (int opt : options) {
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
        if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
            quit = true;
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
            // Other sorting modes can be handled here
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

void selectionSort(std::vector<int>& arr, SDL_Renderer* renderer) {
    for (size_t i = 0; i < arr.size() - 1; ++i) {
        size_t minIndex = i;
        for (size_t j = i + 1; j < arr.size(); ++j) {
            handleEvents();
            if (arr[j] < arr[minIndex]) {
                minIndex = j;
            }
            renderSort(renderer, arr, j, minIndex, "selection");
            SDL_Delay(100);
        }
        std::swap(arr[i], arr[minIndex]);
        renderSort(renderer, arr, i, minIndex, "selection");
        SDL_Delay(100);
    }
}

void insertionSort(std::vector<int>& arr, SDL_Renderer* renderer) {
    for (size_t i = 1; i < arr.size(); ++i) {
        int key = arr[i];
        size_t j = i - 1;

        while (j < arr.size() && arr[j] > key) { 
            handleEvents();
            arr[j + 1] = arr[j];
            renderSort(renderer, arr, i, j + 1, "insertion"); // Highlight the index being inserted and moving index
            j--;
            SDL_Delay(100);
        }
        arr[j + 1] = key;
        renderSort(renderer, arr, i, j + 1, "insertion"); // Highlight the final position of the inserted index
        SDL_Delay(100);
    }
}

void bubbleSort(std::vector<int>& arr, SDL_Renderer* renderer) {
    for (size_t i = 0; i < arr.size() - 1; ++i) {
        for (size_t j = 0; j < arr.size() - i - 1; ++j) {
            handleEvents();
            if (arr[j] > arr[j + 1]) {
                std::swap(arr[j], arr[j + 1]);
            }
            renderSort(renderer, arr, j, j + 1, "bubble");
            SDL_Delay(100);
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
        if (L[i] <= R[j]) {
            arr[k++] = L[i++];
        } else {
            arr[k++] = R[j++];
        }
        renderSort(renderer, arr, k - 1, -1, "merge");
        SDL_Delay(100);
    }

    while (i < n1) {
        handleEvents();
        arr[k++] = L[i++];
        renderSort(renderer, arr, k - 1, -1, "merge");
        SDL_Delay(100);
    }
    while (j < n2) {
        handleEvents();
        arr[k++] = R[j++];
        renderSort(renderer, arr, k - 1, -1, "merge");
        SDL_Delay(100);
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
            if (arr[j] < pivot) {
                ++i;
                std::swap(arr[i], arr[j]);
            }
            renderSort(renderer, arr, j, high, "quick");
            SDL_Delay(100);
        }
        std::swap(arr[i + 1], arr[high]);
        renderSort(renderer, arr, i + 1, high, "quick");
        SDL_Delay(100);
        
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
        SDL_Delay(100); // Delay to visualize the change
        heapify(arr, n, largest, renderer);
    }
}

void heapSort(std::vector<int>& arr, SDL_Renderer* renderer) {
    int n = arr.size();
    // Build heap (rearrange array)
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i, renderer);
        renderSort(renderer, arr, i, -1, "heap"); // Visualize heap formation
        SDL_Delay(100); // Delay to visualize the heap building
    }
    // One by one extract an element from heap
    for (int i = n - 1; i > 0; i--) {
        std::swap(arr[0], arr[i]);
        renderSort(renderer, arr, i, -1, "heap"); // Visualize the array after swap
        SDL_Delay(100); // Delay to visualize the change
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
}

int main() {
    std::cout << "Select two sorting algorithms to visualize:\n";
    std::cout << "1. Selection Sort\n";
    std::cout << "2. Insertion Sort\n";
    std::cout << "3. Bubble Sort\n";
    std::cout << "4. Merge Sort\n";
    std::cout << "5. Quick Sort\n";
    std::cout << "6. Heap Sort\n";
    int firstOption, secondOption;
    
    std::cout << "Enter your first choice (1-6): ";
    std::cin >> firstOption;
    std::cout << "Enter your second choice (1-6): ";
    std::cin >> secondOption;

    if (!init(firstOption, secondOption)) {
        return 1;
    }

    // Start sorting algorithms in separate threads
    std::thread firstThread(executeSorting, firstOption);
    std::thread secondThread(executeSorting, secondOption);

    // Main event loop
    while (!quit) {
        handleEvents();
        SDL_Delay(10); // Small delay to prevent busy-waiting
    }

    // Join threads before exiting
    if (firstThread.joinable()) firstThread.join();
    if (secondThread.joinable()) secondThread.join();

    // Clean up
    close();

    return 0;
}
