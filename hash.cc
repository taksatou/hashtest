#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#include <libhashkit/hashkit.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <set>
#include <map>
#include <stdlib.h>
#include <math.h>

using namespace std;

//#define TEST_RAND
#define TEST_CASES 10000
#define LETTER_NUM 62
static char letters[LETTER_NUM+1] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

static string rand_str(int len, unsigned int seed) {
    char buf[len+1];
    
    srand(seed + 1);
    for (int i = 0; i < len; i++) {
        buf[i] = letters[(int)((double)LETTER_NUM * rand() / (RAND_MAX + 1.0))];
    }
    buf[len] = '\0';
    return string(buf);
}

static set<string> rand_strings;

static void gen_rand_strings(int n)
{
    rand_strings.erase(rand_strings.begin(), rand_strings.end());
    
    for (int i = 0; i < n; ++i) {
        string s = rand_str(10, i);
        
        if (!rand_strings.insert(s).second) {
            cerr << "<" << s << "> is known!" << endl;
            ++n;
        }
    }
}
    
 

class TestHash {
private:
    multiset<uint32_t> results;
    Hashkit h;
    hashkit_hash_algorithm_t hash_type;
    int cases;

    void init(void) {
        results.erase(results.begin(), results.end());
    }
    
public:
    TestHash(int n, hashkit_hash_algorithm_t t) : cases(n), hash_type(t) {
        init();
        h.set_function(t);
    }
    
    void byRand(void) {
        init();
        
        for (set<string>::iterator it = rand_strings.begin(); it != rand_strings.end(); ++it) {
            string s = *it;
            results.insert(h.digest(s));
        }
    }
    
    void bySeq(void) {
        init();
        char buf[256];
        
        for (int i = 0; i < cases; ++i) {
            snprintf(buf, 256, "%010d", i);
            string s(buf);
            results.insert(h.digest(s));
        }
    }


    bool conflict(void) {
        uint32_t last = -1;
        
        for (multiset<uint32_t>::iterator it = results.begin(); it != results.end(); ++it) {
            if (last == *it)
                return true;
            last = *it;
        }
        return false;
    }

    double average(void) {        
        double sum = 0.0;

        for_each(results.begin(), results.end(), sum += boost::lambda::_1);
        return sum / cases;
    }

    double variance() {
        double avr = average();
        double sum = 0.0;

        for_each(results.begin(), results.end(), sum += bind(pow, avr - boost::lambda::_1, 2));
        return sum / cases;
    }

    double deviation(void) {
        return sqrt(variance());
    }

    vector<int> histogram(int n) {
        uint32_t max = *max_element(results.begin(), results.end());
        vector<int> ret;

        ret.assign(n+1, 0);
        for (multiset<uint32_t>::iterator it = results.begin(); it != results.end(); ++it) {
            ret[(int)((double) *it * n / max)] += 1;
        }
        ret.pop_back(); // remove max element
        return ret;
    }
    
    void dump(void) {
        cout << "conflict: " << (conflict() ? 'Y' : '-') << endl;
        cout << "average: " << average() << endl;
        cout << "variance: " << variance() << endl;
        cout << "deviation: " << (long)deviation() << endl;
        vector<int> ret = histogram(10);
        cout << "histogram: " << endl;
        for_each(ret.begin(), ret.end(), cout << boost::lambda::_1 << "\n");
    }

};

void exec_test(pair<hashkit_hash_algorithm_t, string> info)
{
    TestHash test(TEST_CASES, info.first);

    cout << "-----------------" << endl;
    cout << info.second << endl;
#ifdef TEST_RAND
    test.byRand();
    test.dump();
#else
    test.bySeq();
    test.dump();
#endif
}

int main(void)
{
    map<hashkit_hash_algorithm_t, string> types;

    types.insert(make_pair(HASHKIT_HASH_MD5, "HASHKIT_HASH_MD5"));
    types.insert(make_pair(HASHKIT_HASH_CRC, "HASHKIT_HASH_CRC"));
    types.insert(make_pair(HASHKIT_HASH_FNV1_64, "HASHKIT_HASH_FNV1_64"));
    types.insert(make_pair(HASHKIT_HASH_FNV1A_64, "HASHKIT_HASH_FNV1A_64"));
    types.insert(make_pair(HASHKIT_HASH_FNV1_32, "HASHKIT_HASH_FNV1_32"));
    types.insert(make_pair(HASHKIT_HASH_FNV1A_32, "HASHKIT_HASH_FNV1A_32"));
    types.insert(make_pair(HASHKIT_HASH_HSIEH, "HASHKIT_HASH_HSIEH"));
    types.insert(make_pair(HASHKIT_HASH_MURMUR, "HASHKIT_HASH_MURMUR"));
    types.insert(make_pair(HASHKIT_HASH_JENKINS, "HASHKIT_HASH_JENKIN"));
    gen_rand_strings(TEST_CASES);
    
    for_each(types.begin(), types.end(), bind(exec_test, boost::lambda::_1));
    return 0;
}
