#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

#include <files/Allocators.h>

using Vec = std::vector<int, MmapFiles::mmap_allocator<int>>;

std::ostream& operator<<(std::ostream& out, const Vec& v)
{
    std::stringstream ss; 
    ss << "vector is { ";
    for (auto elem : v)
    {
        ss << elem << ",";
    }
    ss.seekp(-1, std::stringstream::end);
    ss << " }";
    
    out << ss.str();

    return out;
}

int main(int argc, char const *argv[])
{
    Vec v1 = { 4,8,1,5,7 }, v2 = { 8,7,5,4,1};

    std::cout << v1 << "\n" << v2 << "\n";

    std::sort(v1.begin(), v1.end());
    std::sort(v2.begin(), v2.end());

    std::cout << v1 << "\n" << v2 << "\n";
    std::cout << "vector are same is " << std::boolalpha << (v1 == v2) << "\n";
    std::cout <<
        "use command: " <<
        "od -t d" << sizeof(int) << " --width=" << sizeof(int) << " FILE_NAME\n";

    return 0;
}
