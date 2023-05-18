#include <iostream>
#include <utility>
#include <vector>

#include <files/unordered_map.h>

using namespace std;

class Solution {
public:
    
    using UMap = MmapFiles::unordered_map_file<int,int>;

    pair<int, int>
    twoSum(vector<int>& nums, int target)
    {
        UMap cache;
            
        for (typename UMap::size_type i = 0; i != nums.size(); ++i)
        {
            auto curr = nums[i];
            
            auto iter = cache.find(curr);
            if (iter != cache.cend())
            {
                return { iter->second,i };
            }     
            
            cache[target - curr] = i;
        }
        
        return { -1,-1 };
    }

};

int main(int argc, char const *argv[])
{
    vector<int> nums = { 2,7,11,15 };
    int target       = 9;

    auto indicies = Solution().twoSum(nums, target);
    std::cout <<
        "nums[" << indicies.first << "]" << 
        " + " <<
        "nums[" << indicies.second << "]" <<
        " = " << 
        target << "\n";

    nums   = { 1,2,3,4,5 };
    target = 10;
    
    indicies = Solution().twoSum(nums, target);
    std::cout <<
        "cannot find two indicies which equal target, got " <<
        "[" << indicies.first <<
        "," <<
        indicies.second << "]" <<
        "\n";

    return 0;
}
