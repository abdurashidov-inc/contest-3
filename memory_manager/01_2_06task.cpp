#include <iostream>
#include "os_mem.h"

char *copy_str(const char *str, size_t str_len) {
    auto new_str = new char[str_len + 1];
    strncpy(new_str, str, str_len);
    new_str[str_len] = '\0';
    return new_str;
}

class Obj {
public:
    char *name = nullptr;
    int name_len;
    Obj **links = nullptr;
    int last_link_idx = -1;
    bool is_root = false;
    bool is_alive = false;

    Obj(const char *obj_name, int obj_name_len = -1) {
        name_len = obj_name_len != -1 ? obj_name_len : (int)strlen(obj_name);
        name = copy_str(obj_name, name_len);
        links = new Obj*[16];
    }

    ~Obj() {
        delete[] name;
        delete[] links;
    }

    int set_link(Obj *obj) {
        if (last_link_idx == 15)
            return 0;
        last_link_idx++;
        if (links == nullptr)
            return 0;
        links[last_link_idx] = obj;
        return 1;
    }
};

class MemoryManager {
private:
    int _free_capacity = 0;
    Obj **_objects = nullptr;
    int last_obj_idx = -1;
    bool _is_created = false;
public:
    MemoryManager() {}
    ~MemoryManager() {
        destroy();
    }

    int init(int size, int num_pages) {
        if (size < 0 || _is_created)
            return 0;
        _free_capacity = size;
        _objects = new Obj*[size];
        _is_created = true;
        return 1;
    }

    int destroy() {
        if (!_is_created)
            return 0;
        for (int i = 0; i <= last_obj_idx; i++)
            delete _objects[i];
        delete[] _objects;
        _free_capacity = 0;
        last_obj_idx = -1;
        _is_created = false;
        return 1;
    }

    int create_object(const char* name) {
        if (!_is_created)
            return 0;

        if (_free_capacity == 0)
            return 0;

        int name_len = (int)strlen(name);

        if (name_len > 32)
            return 0;

        if (_obj_is_exists(name, name_len) != -1)
            return 0;

        int idx = _get_idx_for_new_obj(name);
        for (int i = last_obj_idx; idx <= i ; i--)
            _objects[i + 1] = _objects[i];
        _objects[idx] = new Obj(name, name_len);

        _free_capacity--;
        last_obj_idx++;
        return 1;
    }

    int destroy_object(const char* name) {
        if (!_is_created)
            return 0;

        int name_len = (int)strlen(name);
        int idx = _obj_is_exists(name, name_len);
        if (idx == -1)
            return 0;

        // обнуляем все ссылки, указывающие на удаляемый объект
        for (int i = 0; i <= last_obj_idx; i++) {
            if (i == idx)
                continue;
            for (int j = 0; j <= _objects[i]->last_link_idx; j++) {
                if (_objects[i]->links[j] == _objects[idx])
                    _objects[i]->links[j] = nullptr;
            }
        }

        delete _objects[idx];
        for (int i = idx; i < last_obj_idx; i++) {
            _objects[i] = _objects[i + 1];
        }

        _free_capacity++;
        last_obj_idx--;
        return 1;
    }

    void print_objects(bool only_alive = false) {
        if (!_is_created)
            return;
        for (int i = 0; i <= last_obj_idx; i++) {
            if (only_alive && !_objects[i]->is_alive)
                continue;
            std::cout << _objects[i]->name << std::endl;
        }
    }

    int set_root(const char* name) {
        if (!_is_created)
            return 0;

        int idx = _obj_is_exists(name, (int)strlen(name));
        if (idx == -1)
            return 0;
        if (_objects[idx]->is_root)
            return 0;
        _objects[idx]->is_root = true;
        return 1;
    }

    int link(const char* object1_name, const char* object2_name) {
        if (!_is_created)
            return 0;

        int obj1_idx = _obj_is_exists(object1_name, (int)strlen(object1_name));
        int obj2_idx = _obj_is_exists(object2_name, (int)strlen(object2_name));

        if (obj1_idx == -1 || obj2_idx == -1)
            return 0;

        return _objects[obj1_idx]->set_link(_objects[obj2_idx]);
    }

    void collect_live_objects() {
        if (!_is_created)
            return;

        for (int i = 0; i <= last_obj_idx; i++)
            _objects[i]->is_alive = false;

        for (int i = 0; i <= last_obj_idx; i++)
            if (_objects[i]->is_root && !_objects[i]->is_alive)
                _mark_obj_as_live(_objects[i]);

        print_objects(true);
    }

private:
    int _obj_is_exists(const char* name, int name_len) {
        for (int i = 0; i <= last_obj_idx; i++)
            if (_objects[i]->name_len == name_len && strncmp(_objects[i]->name, name, name_len) == 0)
                return i;
        return -1;
    }

    int _get_idx_for_new_obj(const char* name) {
        int l = 0, r = last_obj_idx + 1, mid;
        while (l < r) {
            mid = (r + l) >> 1;
            if (strcmp(name, _objects[mid]->name) > 0)
                l = mid + 1;
            else
                r = mid;
        }
        return l;
    }

    void _mark_obj_as_live(Obj *obj) {
        if (obj == nullptr)
            return;
        if (obj->is_alive)
            return;
        obj->is_alive = true;
        for (int i = 0; i <= obj->last_link_idx; i++) {
            _mark_obj_as_live(obj->links[i]);
        }
    }
};

MemoryManager memory_manager;

void setup_memory_manager(memory_manager_t *mm) {
    mm->create = [](int size, int num_pages) {return memory_manager.init(size, num_pages);};
    mm->destroy = []() {return memory_manager.destroy();};
    mm->create_object = [](const char* name) {return memory_manager.create_object(name);};
    mm->destroy_object = [](const char* name) {return memory_manager.destroy_object(name);};
    mm->print_objects = []() {return memory_manager.print_objects();};
    mm->set_root = [](const char* name) {return memory_manager.set_root(name);};
    mm->link = [](const char* object1_name, const char* object2_name) {return memory_manager.link(object1_name, object2_name);};
    mm->collect_live_objects = []() {return memory_manager.collect_live_objects();};
}
