#include <iostream>

class MyArray {
private:
    int data[5] = {1, 2, 3, 4, 5};
    int value = 10;
public:
    int& operator[](int index) {
        return data[index];
    }

    int& operator()(int index) {
        return data[index];
    }
};

int main() {
    MyArray arr;
    
    // Using the overridden operator[]
    std::cout << arr[2] << std::endl; // Output: 3
    
    // Modifying the value using operator[]
    arr[2] = 10;
    std::cout << arr[2] << std::endl; // Output: 10

    arr(3) = 20;
    
    return 0;
}