// Example program
#include <iostream>
#include <string>
#include <unordered_map>

typedef void (*func)(void);

void func1()
{
    std::cout << "Function 1\n";
}

void func2()
{
    std::cout << "Function 2\n";
}

int main()
{
  std::unordered_map<int, func> map;

  map.insert( {0, func1} );
  map.insert( {6, func2} );

  std::cout << map[6] << std::endl;

  map[6]();
}
