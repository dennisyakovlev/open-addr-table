#include <sstream>
#include <iostream>

#include <files/unordered_map.h>
#include <tests_support/CustomString.h>

/*  16 is maximum number of characters in a word. So max len
    word is 15 chars.
*/
using Map  = MmapFiles::unordered_map_file<int, MyString<16>>;
using Size = typename Map::size_type;

constexpr Size max_words = 20;

std::string words = "Some words which i'll sub later for something more. "
                    "Above task ended up in the backlog.";
std::string name  = "paragraph.txt";

int main(int argc, char const *argv[])
{
    std::string op(argv[1]);

    if (op == "read")
    {
        Map file(name, max_words, true);
        for (auto& pair : file)
        {
            std::cout << "(" << pair.first << "," << pair.second << ")\n";
        }
    }
    else if (op == "write")
    {
        Map file(name);
        file.reserve(max_words);

        /*  Write one word per element. Know that open address algo is
            same as array if no collisions. Use index "i" as key, so there
            will be no collisions.
        */
        std::stringstream ss(words);
        for (Size i = 0; ss >> file[i].M_str; ++i);

        std::cout <<
            "use command: " <<
            "od -t c --width=" << sizeof(typename Map::element) << " " << name << "\n";
    }

    return 0;
}

