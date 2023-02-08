#include <utility>

#include <gtest/gtest.h>

#include <files/Files.h>
#include <tests_support/SpecialHash.h>
#include <tests_support/StrictOperation.h>
#include <tests_support/Vars.h>

using File = MmapFiles::unordered_map_file<SpecialHash,std::size_t>;

class TestRehash :
    public testing::Test,
    public StrictOperation<File>
{
protected:

    TestRehash() :
        StrictOperation(File(unit_test_file))
    {
    }

};

/*  Below tests have comments of the format of two
    cols of numbers with two differnt representations
    of the container at different points in the test.



    First col is the indicies in the container.

    Second col is where, if working correctly, the
    hash values in the container should be loacted.
    The format (A,B) is
        B - the hash value of the inserted key
        A - B modded by number of buckets in container
    


    The first representation is, if working correctly,
    the state of the container directly after insert
    is called.

    The second representation is, if working
    correctly, the state of the container directly
    after calling rehash for the second time.
*/

TEST_F(TestRehash, A)
{
    /*  0 (4,9)
        1 (1,11)
        2 (2,2)
        3 (2,22)
        4 (3,13)

        0
        1 (1,11)
        2 (2,2)
        3 (2,22)
        4 (3,13)
        5
        6
        7
        8
        9 (9,9)
    */
    
    cont.bucket_choices({5,10});
    cont.reserve(5);

    insert({2,13,22,9,11});

    cont.rehash(10);

    erase_and_check(4);
    erase_and_check(0);
    erase_and_check(1);
    erase_and_check(2);
    erase_and_check(3);
}

TEST_F(TestRehash, B)
{
    /*  0 (0,80)
        1
        2
        3
        4
        5 (5,37)
        6 (5,21)
        7

        0
        1
        2
        3
        4
        5 (5,80)
        6 (6,21)
        7 (7,37)
        8
        9
        10
        11
        12
        13
        14
    */

    cont.bucket_choices({8,15});
    cont.reserve(8);

    insert({80,37,21});

    cont.rehash(15);

    erase_and_check(0);
    erase_and_check(1);
    erase_and_check(2);

}

TEST_F(TestRehash, C)
{
    /*  0 (4,14)
        1 (1,11)
        2 (2,2)
        3 (2,22)
        4 (3,13)

        0 
        1 (1,11)
        2 (2,2)
        3 (2,22)
        4 (3,13)
        5 (4,14)
        6
        7
        8
        9
    */

    cont.bucket_choices({5,10});
    cont.reserve(5);

    insert({14,13,22,2,11});

    cont.rehash(10);

    erase_and_check(3);
    erase_and_check(2);
    erase_and_check(0);
    erase_and_check(1);
    erase_and_check(4);
}

TEST_F(TestRehash, D)
{
    /*  0 (0,0)
        1 (1,11)
        2 (2,12)
        3
        4 (4,9)

        0 (0,0)
        1 (1,9)
        2
        3 (3,11)
        4 (4,12)
        5
        6
        7
    */

    cont.bucket_choices({5,8});
    cont.reserve(5);

    insert({0,11,12,9});

    cont.reserve(8);

    erase_and_check(0);
    erase_and_check(1);
    erase_and_check(2);
    erase_and_check(3);
}

TEST_F(TestRehash, E)
{
    /*  0 (0,80)
        1
        2
        3
        4
        5 (5,37)
        6
        7 (7,231)

        0
        1
        2
        3
        4
        5 (5,80)
        6 (6,231)
        7 (7,37)
        8
        9
        10
        11
        12
        13
        14
    */

    cont.bucket_choices({8,15});
    cont.reserve(8);

    insert({80,231,21});

    cont.rehash(15);

    erase_and_check(0);
    erase_and_check(1);
    erase_and_check(2);
}

TEST_F(TestRehash, F)
{
    /*  0
        1 (1,37)
        2 (1,667)
        3 (2,278)
        4 (4,82)
        5 (4,142)

        0
        1
        2
        3
        4
        5
        7  (7,37)
        8  (7,82)
        9  (7,667)
        10 (7,142)
        11 (8,278)
        12
        13
        14
    */

    cont.bucket_choices({6,15});
    cont.reserve(6);

    insert({278,667,82,142,37});

    cont.rehash(15);

    erase_and_check(0);
    erase_and_check(1);
    erase_and_check(2);
    erase_and_check(3);
    erase_and_check(4);
}

TEST_F(TestRehash, G)
{
    /*  0 (5,189)
        1 (5,285)
        2 (5,69)
        3 (1,153)
        4 (1,9)
        5 (5,45)
        6 (5,117)
        7 (5,165)

        0 (9,9)
        1 (9,165)
        2 (9,189)
        3 (9,285)
        4 (9,69)
        5
        6
        7
        8
        9  (9,45)
        10 (9,153)
        11 (9,117)
    */

    cont.bucket_choices({8,12});
    cont.reserve(8);

    insert({189,285,69,153,165,117,45,9});

    erase_and_check(0);
    erase_and_check(1);
    erase_and_check(2);
    erase_and_check(3);
    erase_and_check(4);
    erase_and_check(5);
    erase_and_check(6);
    erase_and_check(7);

}

/*  NOTE: need downsizing tests
*/
