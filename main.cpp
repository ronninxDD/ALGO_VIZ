#define SDL_MAIN_HANDLED //Ensures SDL doesn't redefine main
#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <string>
#include <algorithm>

using namespace std;

const int WINDOW_WIDTH = 800; //window width
const int WINDOW_HEIGHT = 300; //window height
const int BAR_WIDTH = 10;  //width of each bar in visualization

SDL_Window* windows[6] = {nullptr}; // array of pointers to SDL windows 
SDL_Renderer* renderers[6] = {nullptr}; //array of pointers to SDL renderers 

bool quit = false; //flag to control the quitting of program
int delay = 100; // Default delay for speed


//FUnction to initialize SDL and create windows and renderers based on user options

bool init(int options[], int count) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {//Initialize SDL
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false; //Return false if initializatio nfails
    }

    string titles[] = {"Selection Sort Visualizer", "Insertion Sort Visualizer", "Bubble Sort Visualizer", "Merge Sort Visualizer", "Quick Sort Visualizer", "Heap Sort Visualizer"};

    for (int k = 0; k < count; ++k) {
        int opt = options[k];
        if (opt < 1 || opt > 6) { //check for invalid option 
            std::cerr << "Invalid option!" << std::endl;
            return false; //return false if invalid option
        }

        int i = opt - 1; // Adjust option to index
        windows[i] = SDL_CreateWindow(titles[i].c_str(), SDL_WINDOWPOS_CENTERED + (i % 2) * WINDOW_WIDTH, SDL_WINDOWPOS_CENTERED + (i / 2) * WINDOW_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
        if (!windows[i]) {//Create window and check for failure
            std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            SDL_Quit();
            return false; //return false if window creation fails
        }
        renderers[i] = SDL_CreateRenderer(windows[i], -1, SDL_RENDERER_ACCELERATED);
        if (!renderers[i]) {// create renderer and check for failure
            std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            SDL_Quit();
            return false; // return false if renderer creation fails
        }
    }
    return true; //return true if initialization succeeds 
}


// FUnction to close and destroy all SDL windows and renderers 
void close() {
    for (int i = 0; i < 6; ++i) {
        if (renderers[i]) SDL_DestroyRenderer(renderers[i]);//destroy renderer if it  exists
        if (windows[i]) SDL_DestroyWindow(windows[i]); //destroy windows if exists 
    }
    SDL_Quit();
}


//function to handel events 
void handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
            quit = true;// set quit flag if quit event or escape key is pressed 
        }
    }
}


//Function to render the sorting visualization
void renderSort(SDL_Renderer* renderer, const std::vector<int>& arr, size_t currentIndex, size_t secondIndex, const std::string& mode) {
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF); // Black color for bars
    SDL_RenderClear(renderer); //clear the renderer
    
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


//Function to perform and visualize selection sort 
void selectionSort(std::vector<int>& arr, SDL_Renderer* renderer) {
    for (size_t i = 0; i < arr.size() - 1; ++i) {
        size_t minIndex = i;
        for (size_t j = i + 1; j < arr.size(); ++j) {
            handleEvents();
            if (arr[j] < arr[minIndex]) {
                minIndex = j; //update min index if smaller element is found
            }
            renderSort(renderer, arr, j, minIndex, "selection"); //render the current state of array
            SDL_Delay(delay); //delay for visualization
        }
        std::swap(arr[i], arr[minIndex]); //swap the min element with first element
        renderSort(renderer, arr, i, minIndex, "selection");//render state after the swap
        SDL_Delay(delay);
    }
}


// insertion sort visualization
void insertionSort(std::vector<int>& arr, SDL_Renderer* renderer) {
    for (size_t i = 1; i < arr.size(); ++i) {
        int key = arr[i];
        size_t j = i - 1;

        while (j < arr.size() && arr[j] > key) { 
            handleEvents();
            arr[j + 1] = arr[j]; //Shift elements to the right
            renderSort(renderer, arr, i, j + 1, "insertion"); // Highlight the index being inserted and moving index
            j--;
            SDL_Delay(delay);
        }
        arr[j + 1] = key; // Insert the key at the correct position
        renderSort(renderer, arr, i, j + 1, "insertion"); // Highlight the final position of the inserted index
        SDL_Delay(delay);
    }
}


// Function to perform and visualize bubble sort

void bubbleSort(std::vector<int>& arr, SDL_Renderer* renderer) {
    for (size_t i = 0; i < arr.size() - 1; ++i) {
        for (size_t j = 0; j < arr.size() - i - 1; ++j) {
            handleEvents();
            if (arr[j] > arr[j + 1]) {
                std::swap(arr[j], arr[j + 1]); // Swap if the element is greater than the next element
            }
            renderSort(renderer, arr, j, j + 1, "bubble"); // Render the current state of the array
            SDL_Delay(delay); // Delay for visualization
        }
    }
}


// Function to merge two halves of an array
void merge(std::vector<int>& arr, int left, int mid, int right, SDL_Renderer* renderer) {
    int n1 = mid - left + 1;
    int n2 = right - mid;\\ render the current state of the array            sdl_delay(delay); \\ Delay for visualization

    std::vector<int> L(n1), R(n2);
    for (int i = 0; i < n1; ++i) L[i] = arr[left + i]; // Copy left half
    for (int j = 0; j < n2; ++j) R[j] = arr[mid + 1 + j]; // Copy right half

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        handleEvents();
        if (L[i] <= R[j]) {
            arr[k++] = L[i++];
        } else {
            arr[k++] = R[j++];
        }
        renderSort(renderer, arr, k - 1, -1, "merge");// Render the merging process
        SDL_Delay(delay);
    }

    while (i < n1) {
        handleEvents();
        arr[k++] = L[i++];
        renderSort(renderer, arr, k - 1, -1, "merge");// Render the remaining left half
        SDL_Delay(delay);
    }
    while (j < n2) {
        handleEvents();
        arr[k++] = R[j++];
        renderSort(renderer, arr, k - 1, -1, "merge");// Render the remaining right half
        SDL_Delay(delay);
    }
}

// Function to perform and visualize merge sort
void mergeSort(std::vector<int>& arr, int left, int right, SDL_Renderer* renderer) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid, renderer); // Sort first half
        mergeSort(arr, mid + 1, right, renderer); // Sort second half
        merge(arr, left, mid, right, renderer); // Merge the two halves
    } 
}

// Function to perform and visualize quick sort
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
            renderSort(renderer, arr, j, high, "quick"); // Render the current state of the array
            SDL_Delay(delay);
        }
        std::swap(arr[i + 1], arr[high]);
        renderSort(renderer, arr, i + 1, high, "quick"); //// Render the state after placing pivot
        SDL_Delay(delay);
        
        quickSort(arr, low, i, renderer);//// Recursively sort elements before partition
        quickSort(arr, i + 2, high, renderer);// Recursively sort elements before partition
    }
}


// Function to heapify a subtree rooted at index i
void heapify(std::vector<int>& arr, int n, int i, SDL_Renderer* renderer) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n && arr[left] > arr[largest]) largest = left;
    if (right < n && arr[right] > arr[largest]) largest = right;

    if (largest != i) {
        std::swap(arr[i], arr[largest]);
        renderSort(renderer, arr, largest, i, "heap"); // Render the current state of the array
        SDL_Delay(delay); // Delay to visualize the change
        heapify(arr, n, largest, renderer);
    }
}

// Function to perform and visualize heap sort
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
// Function to execute and visualize sorting algorithms based on user options
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
//menu
void showMenu() {
    std::cout << "Main Menu:\n";
    std::cout << "1. One Visualization\n";
    std::cout << "2. Multiple Visualizations\n";
    std::cout << "3. Change Speed\n";
    std::cout << "4. Exit\n";
    std::cout << "Enter your choice: ";
}
//speed options
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

                if (numSorts < 1 || numSorts > 6) {
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
