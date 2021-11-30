#include <iostream>
#include "os_file.h"

static const int NAME_MAX_LEN = 32;
static const int MAX_NESTING = 10;
static bool TEST_PASSED = true;

void print(char *str, int expected_res, int res) {
    if (res == expected_res) {
        std::cout << str << res << "; \tOK" << std::endl;
    } else {
        std::cout << str << res << "; \tFAIL" << std::endl;
        TEST_PASSED = false;
    }
}

void print(char *str, char *expected_res, char *res) {
    if (strcmp(res, expected_res) == 0) {
        std::cout << str << res << "; \tOK" << std::endl;
    } else {
        std::cout << str << res << "; \tFAIL" << std::endl;
        TEST_PASSED = false;
    }
}

int main() {
    file_manager_t fm;
    setup_file_manager(&fm);

    auto dst = new char[(NAME_MAX_LEN + 1)*MAX_NESTING];

    print((char *)"fm.free_space(): result = ", 0, fm.free_space());
    print((char *)"fm.create(-100): result = ", 0, fm.create(-100));
    print((char *)"fm.create(100): result = ", 1, fm.create(100));
    print((char *)"fm.remove(\"/\", 0): result = ", 0, fm.remove((char *)"/", 0));
    print((char *)"fm.remove(\"/\", 1): result = ", 0, fm.remove((char *)"/", 1));
    print((char *)"fm.create(100) [already created]: result = ", 0, fm.create(100));
    print((char *)"fm.destroy(): result = ", 1, fm.destroy());
    print((char *)"fm.destroy() [already destroyed]: result = ", 0, fm.destroy());
    print((char *)"fm.free_space(): result = ", 0, fm.free_space());

    print((char *)"fm.create(100): result = ", 1, fm.create(100));

    print((char *)"fm.create_dir(\"d1/d2\") [./d1 doesn't exist]: result = ", 0, fm.create_dir("d1/d2"));
    print((char *)"fm.create_dir(\"d?1\") [invalid name]: result = ", 0, fm.create_dir("d?1"));
    print((char *)"fm.create_dir(\"../d1\") [current_dir='/']: result = ", 0, fm.create_dir("../d1"));
    print((char *)"fm.create_dir(\"d1\"): result = ", 1, fm.create_dir("d1"));
    print((char *)"fm.create_dir(\"d1\") [already exists]: result = ", 0, fm.create_dir("d1"));
    print((char *)"fm.change_dir(\"d2\") [./d2 doesn't exist]: result = ", 0, fm.change_dir("d2"));

    print((char *)"fm.change_dir(\"..\"): result = ", 0, fm.change_dir(".."));
    fm.get_cur_dir(dst);
    print((char *)"fm.get_cur_dir(dst): result = ", (char *)"/", dst);
    print((char *)"fm.change_dir(\"d1\"): result = ", 1, fm.change_dir("d1"));
    fm.get_cur_dir(dst);
    print((char *)"fm.get_cur_dir(dst): result = ", (char *)"/d1", dst);
    print((char *)"fm.change_dir(\".\"): result = ", 1, fm.change_dir("."));
    fm.get_cur_dir(dst);
    print((char *)"fm.get_cur_dir(dst): result = ", (char *)"/d1", dst);
    print((char *)"fm.create_dir(\"../d2\"): result = ", 1, fm.create_dir("../d2"));
    print((char *)"fm.create_dir(\"../d2/d2.1\"): result = ", 1, fm.create_dir("../d2/d2.1"));
    print((char *)"fm.create_dir(\"../d2/d2.1/d2.2\"): result = ", 1, fm.create_dir("../d2/d2.1/d2.2"));

    print((char *)"fm.change_dir(\"../d2/d2.1/d2.2\"): result = ", 1, fm.change_dir("../d2/d2.1/d2.2"));
    fm.get_cur_dir(dst);
    print((char *)"fm.get_cur_dir(dst): result = ", (char *)"/d2/d2.1/d2.2", dst);

    print((char *)"fm.free_space(): result = ", 100, fm.free_space());
    print((char *)"fm.create_file(\"./f2.3\", 10): result = ", 1, fm.create_file("./f2.3", 10));
    print((char *)"fm.create_file(\"./f2.3?\", 10): result = ", 0, fm.create_file("./f2.3?", 10));
    print((char *)"fm.free_space(): result = ", 90, fm.free_space());
    print((char *)"fm.change_dir(\"f2.3\"): result = ", 0, fm.change_dir("f2.3"));
    print((char *)"fm.create_dir(\"./f2.3/d2.4\"): result = ", 0, fm.create_dir("./f2.3/d2.4"));

    print((char *)"fm.create_file(\"./f2.3\", 10) [already exists]: result = ", 0, fm.create_file("./f2.3", 10));
    print((char *)"fm.create_file(\"./f2.3.1\", 91): result = ", 0, fm.create_file("./f2.3.1", 91));

    print((char *)"fm.change_dir(\"/d1\"): result = ", 1, fm.change_dir("/d1"));
    print((char *)"fm.create_dir(\"/d3\"): result = ", 0, fm.create_dir(".."));

    print((char *)"fm.create_file(\"..\", 20): result = ", 0, fm.create_file("..", 20));
    print((char *)"fm.create_file(\"../d2/.\", 20): result = ", 0, fm.create_file("../d2/.", 20));
    print((char *)"fm.create_file(\".\", 20): result = ", 0, fm.create_file(".", 20));
    print((char *)"fm.free_space(): result = ", 90, fm.free_space());

    print((char *)"fm.remove(\"../d2/d2.1\", 0): result = ", 0, fm.remove("../d2/d2.1", 0));
    print((char *)"fm.remove(\"../d2/d2.1\", 1): result = ", 1, fm.remove("../d2/d2.1", 1));
    print((char *)"fm.free_space(): result = ", 100, fm.free_space());
    print((char *)"fm.create_dir(\"..\"): result = ", 0, fm.create_dir(".."));
    print((char *)"fm.create_dir(\"../d2/.\"): result = ", 0, fm.create_dir("../d2/."));
    print((char *)"fm.create_dir(\".\"): result = ", 0, fm.create_dir("."));

    print((char *)"fm.remove(\"/d1?\", 1): result = ", 0, fm.remove("/d1?", 1));
    print((char *)"fm.remove(\".\", 1): result = ", 0, fm.remove(".", 1));
    print((char *)"fm.remove(\"..\", 1): result = ", 0, fm.remove("..", 1));
    fm.get_cur_dir(dst);
    print((char *)"fm.get_cur_dir(dst): result = ", (char *)"/d1", dst);

    print((char *)"fm.create_dir(\"/d3\"): result = ", 1, fm.create_dir("/d3"));
    print((char *)"fm.create_dir(\"/d3/d3.1\"): result = ", 1, fm.create_dir("/d3/d3.1"));
    print((char *)"fm.create_dir(\"../d3/d3.1/d3.2\"): result = ", 1, fm.create_dir("../d3/d3.1/d3.2"));

    print((char *)"fm.create_dir(\"/d1/d1.1\"): result = ", 1, fm.create_dir("/d1/d1.1"));
    print((char *)"fm.change_dir(\"d1.1\"): result = ", 1, fm.change_dir("d1.1"));
    fm.get_cur_dir(dst);
    print((char *)"fm.get_cur_dir(dst): result = ", (char *)"/d1/d1.1", dst);

    print((char *)"fm.remove(\"../../d3/d3.1/d3.2\", 0): result = ", 1, fm.remove("../../d3/d3.1/d3.2", 0));

    print((char *)"fm.remove(\"/d3/d3.1\", 0): result = ", 1, fm.remove("/d3/d3.1", 0));

    print((char *)"fm.remove(\"/d1\", 0): result = ", 0, fm.remove("/d1", 0));
    print((char *)"fm.remove(\"../d1.1\", 0): result = ", 1, fm.remove("../d1.1", 0));
    fm.get_cur_dir(dst);
    print((char *)"fm.get_cur_dir(dst): result = ", (char *)"/", dst);

    print((char *)"fm.create_file(\"\", 10): result = ", 0, fm.create_file("", 10));
    print((char *)"fm.create_dir(\"\"): result = ", 0, fm.create_dir(""));
    print((char *)"fm.change_dir(\"\"): result = ", 0, fm.change_dir(""));

    print((char *)"fm.create_dir(\"./dd\"): result = ", 1, fm.create_dir("./dd"));
    print((char *)"fm.create_file(\"./ff\", 10): result = ", 1, fm.create_file("./ff", 10));
    print((char *)"fm.remove(\"./dd\", 1): result = ", 1, fm.remove("./dd", 1));

    print((char *)"fm.destroy(): result = ", 1, fm.destroy());

    // delete working_dir test
    print((char *)"fm.create(100): result = ", 1, fm.create(100));
    print((char *)"fm.create_dir(\"/d1\"): result = ", 1, fm.create_dir("/d1"));
    print((char *)"fm.create_dir(\"/d1/d2\"): result = ", 1, fm.create_dir("/d1/d2"));
    print((char *)"fm.change_dir(\"/d1/d2\"): result = ", 1, fm.change_dir("/d1/d2"));
    fm.get_cur_dir(dst);
    print((char *)"fm.get_cur_dir(dst): result = ", (char *)"/d1/d2", dst);
    print((char *)"fm.remove(\"/d1\", 1): result = ", 1, fm.remove((char *)"/d1", 1));
    fm.get_cur_dir(dst);
    print((char *)"fm.get_cur_dir(dst): result = ", (char *)"/", dst);
    print((char *)"fm.destroy(): result = ", 1, fm.destroy());

    // fill capacity of object children test
    print((char *)"fm.create(100): result = ", 1, fm.create(100));
    char file_name[] = "/fXX";
    for (int i = 1; i < 12; i++) {
        file_name[2] = (int)(i/10) + '0';
        file_name[3] = (i%10) + '0';
        std::string str = "fm.create_file(\"/f" + std::to_string(i) + "\", 1): result = ";
        print((char *)str.c_str(), 1, fm.create_file(file_name, 1));
    }
    print((char *)"fm.destroy(): result = ", 1, fm.destroy());

    // test by "Я гений" @iftixor454 :)
    print((char *)"fm.create(100): result = ", 1, fm.create(100));
    print((char *)"fm.create_dir(\"/d1\"): result = ", 1, fm.create_dir("/d1"));
    print((char *)"fm.create_dir(\"d1/.././d4\"): result = ", 1, fm.create_dir("d1/.././d4"));
    print((char *)"fm.change_dir(\"/d1/.././d4\"): result = ", 1, fm.change_dir("/d1/.././d4"));
    fm.get_cur_dir(dst);
    print((char *)"fm.get_cur_dir(dst): result = ", (char *)"/d4", dst);
    print((char *)"fm.destroy(): result = ", 1, fm.destroy());

    print((char *)"TEST IS PASSED: ", true, TEST_PASSED);

    delete[] dst;
    return 1;
}
