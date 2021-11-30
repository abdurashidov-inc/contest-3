#include <iostream>
#include "os_mem.h"

void print(char *str, int expected_res, int res) {
    if (res == expected_res) std::cout << str << res << "; \tOK" << std::endl;
    else std::cout << str << res << "; \tFAIL" << std::endl;
}

int main() {
    memory_manager_t mm;
    setup_memory_manager(&mm);

    print((char *)"mm.create_object(\"obj1\"): result = ", 0, mm.create_object("obj1"));
    print((char *)"mm.destroy_object(\"obj1\"): result = ", 0, mm.destroy_object("obj1"));

    print((char *)"mm.print_objects(): [N/A] ", 0, 0);
    mm.print_objects();

    print((char *)"mm.set_root(\"obj1\"): result = ", 0, mm.set_root("obj1"));
    print((char *)"mm.link(\"obj1\", \"obj2\"): result = ", 0, mm.link("obj1", "obj2"));

    print((char *)"mm.collect_live_objects(): [N/A] ", 0, 0);
    mm.collect_live_objects();


    print((char *)"mm.create(-10, 0): result = ", 0, mm.create(-10, 0));
    print((char *)"mm.create(10, 0): result = ", 1, mm.create(10, 0));
    print((char *)"mm.create(10, 0) [already created]: result = ", 0, mm.create(10, 0));

    print((char *)"mm.destroy(): result = ", 1, mm.destroy());
    print((char *)"mm.destroy() [already destroyed]: result = ", 0, mm.destroy());


    print((char *)"mm.create(6, 0): result = ", 1, mm.create(6, 0));

    print((char *)"mm.create_object(\"obj1\"): result = ", 1, mm.create_object("obj1"));
    print((char *)"mm.create_object(\"obj1\") [already exists]: result = ", 0, mm.create_object("obj1"));
    print((char *)"mm.create_object(\"obj2\"): result = ", 1, mm.create_object("obj2"));
    print((char *)"mm.create_object(\"obj3\"): result = ", 1, mm.create_object("obj3"));
    print((char *)"mm.create_object(\"obj5\"): result = ", 1, mm.create_object("obj5"));
    print((char *)"mm.create_object(\"obj4\"): result = ", 1, mm.create_object("obj4"));
    print((char *)"mm.create_object(\"a-obj6\"): result = ", 1, mm.create_object("a-obj6"));

    print((char *)"mm.create_object(\"obj7\") [_free_capacity == 0]: result = ", 0, mm.create_object("obj7"));

    print((char *)"mm.set_root(\"obj0\") [doesn't exist]: result = ", 0, mm.set_root("obj0"));
    print((char *)"mm.set_root(\"obj1\"): result = ", 1, mm.set_root("obj1"));
    print((char *)"mm.set_root(\"obj1\"): result = ", 0, mm.set_root("obj1"));

    print((char *)"mm.print_objects(): [stdout: a-obj6, obj1, obj2, obj3, obj4, obj5] ", 0, 0);
    mm.print_objects();

    print((char *)"mm.collect_live_objects(): [stdout: obj1(*)] ", 0, 0);
    mm.collect_live_objects();

    print((char *)"mm.link(\"obj1\", \"obj0\"): result = ", 0, mm.link("obj1", "obj0"));
    print((char *)"mm.link(\"obj1\", \"obj1\"): result = ", 1, mm.link("obj1", "obj1"));
    print((char *)"mm.link(\"obj1\", \"obj3\"): result = ", 1, mm.link("obj1", "obj3"));
    print((char *)"mm.link(\"obj3\", \"obj4\"): result = ", 1, mm.link("obj3", "obj4"));
    print((char *)"mm.link(\"obj5\", \"obj4\"): result = ", 1, mm.link("obj5", "obj4"));
    print((char *)"mm.link(\"a-obj6\", \"obj1\"): result = ", 1, mm.link("a-obj6", "obj1"));

    print((char *)"mm.collect_live_objects(): [stdout: obj1(*), obj3, obj4] ", 0, 0);
    mm.collect_live_objects();

    print((char *)"mm.set_root(\"obj2\"): result = ", 1, mm.set_root("obj2"));

    print((char *)"mm.destroy_object(\"obj1\")[stdout: obj2(*)] ", 1, mm.destroy_object("obj1"));

    print((char *)"mm.collect_live_objects(): [stdout: obj2(*)] ", 0, 0);
    mm.collect_live_objects();

    print((char *)"mm.print_objects(): [stdout: a-obj6, obj2, obj3, obj4, obj5] ", 0, 0);
    mm.print_objects();

    print((char *)"mm.destroy_object(\"obj2\"): result = ", 1, mm.destroy_object("obj2"));
    print((char *)"mm.destroy_object(\"obj2\") [doesn't exist]: result = ", 0, mm.destroy_object("obj2"));

    return 1;
}
