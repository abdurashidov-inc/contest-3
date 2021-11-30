#include "os_file.h"

// todo: нужен ли _shorten_children_capacity()?

static const int CAPACITY_STEP = 10;

int get_nesting_lvl(const char *str, int len) {
    int nesting_lvl = 1;
    for (int i = 0; i < len; i++)
        if (str[i] == '/')
            nesting_lvl++;
    return nesting_lvl;
}

char *cut_n_chars(const char *str, int str_len, int n) {
    auto new_str = new char[str_len - n + 1];
    for (int i = n; i < str_len; i++)
        new_str[i-n] = str[i];
    new_str[str_len - n] = '\0';
    return new_str;
}

bool str_is_valid(const char *str, bool is_path = false, int str_len = -1) {
    if (str_len == -1)
        str_len = (int)strlen(str);
    if (str_len <= 0)
        return false;
    for (int i = 0; i < str_len; i++)
        if (!('A' <= str[i] && str[i] <= 'Z') && !('a' <= str[i] && str[i] <= 'z') &&
            !('0' <= str[i] && str[i] <= '9') && str[i] != '_' &&str[i] != '.')
            if (str[i] != '/' && !is_path)
                return false;
    return true;
}

bool is_special(const char *str, int str_len) {
    if (
            str_len == 1 && str[0] == '.' ||  // "."
            str_len == 2 && str[str_len - 1] == '.' && str[str_len - 2] == '.' ||  // ".."
            str_len >= 2 && str[str_len - 1] == '.' && str[str_len - 2] == '/' ||  // "*/."
            str_len >= 3 && str[str_len - 1] == '.' && str[str_len - 2] == '.' && str[str_len - 2] == '/'  // "*/.."
            ) {
        return true;
    }
    return false;
}

char *copy_str(const char *str, size_t str_len) {
    auto new_str = new char[str_len + 1];
    strncpy(new_str, str, str_len);
    new_str[str_len] = '\0';
    return new_str;
}

// =====================================================================

class Path {
public:
    int path_len;
    char *path;
    int nesting_lvl = 0;

    Path(char *str, int str_len = -1) {
        _init(str, str_len);
    }

    ~Path() {
        delete[] path;
    }

    bool up_1_lvl() {
        if (path_len == 1)
            return false;

        auto old_path_len = path_len;
        auto old_path = path;

        path_len = old_path_len - 1;
        for (int i = path_len; ; i--) {
            if (path[i] == '/')
                break;
            path_len--;
        }

        path = new char[path_len + 1];
        strncpy(path, old_path, path_len);
        path[path_len] = '\0';

        for (int i = 0; i < path_len; i++)
            path[i] = old_path[i];

        nesting_lvl--;
        delete[] old_path;
        return true;
    }

    bool cmd_cd(char* path2, int path2_len, bool del = false) {
        if (path2[0] == '/') {
            _init((char *)"/", 1, true);

            if (path2_len == 1)
                return true;

            auto new_path = cut_n_chars(path2, path2_len, 1);
            if (del)
                delete[] path2;
            return cmd_cd(new_path, path2_len - 1, true);
        } else if (path2_len == 1 && path2[0] == '.' || path2_len >= 2 && path2[0] == '.' && path2[1] == '/') {
            if (path2_len == 1 || path2_len == 2) {
                if (del)
                    delete[] path2;
                return true;
            }
            auto new_path = cut_n_chars(path2, path2_len, 2);
            if (del)
                delete[] path2;
            return cmd_cd(new_path, path2_len - 2, true);
        } else if (path2_len == 2 && path2[0] == '.' && path2[1] == '.' || path2_len >= 3 && path2[0] == '.' && path2[1] == '.' && path2[2] == '/') {
            if (!up_1_lvl())
                return false;
            if (path2_len == 2 || path2_len == 3) {
                if (del)
                    delete[] path2;
                return true;
            }
            auto new_path = cut_n_chars(path2, path2_len, 3);
            if (del)
                delete[] path2;
            return cmd_cd(new_path, path2_len - 3, true);
        } else {
            if (!str_is_valid(path2, true, path2_len))
                return false;

            int cut_len = 0;
            for (; cut_len < path2_len; cut_len++) {
                if (path2[cut_len] == '/') {
                    cut_len++;
                    break;
                }
            }

            if (cut_len == path2_len) {
                _append(path2, path2_len);
                if (del)
                    delete[] path2;
                return true;
            }

            _append(path2, cut_len - 1);

            auto new_path = cut_n_chars(path2, path2_len, cut_len);
            if (del)
                delete[] path2;
            return cmd_cd(new_path, path2_len - cut_len, true);
        }
    }

    char *get_parent_name_by_lvl(int lvl) const {
        if (lvl > nesting_lvl || lvl == 0)
            return nullptr;

        int counter = 0;
        int start = 0, end = 0;

        for (int i = 0; i < path_len - 1; i++) {
            if (path[i] == '/') {
                counter++;
                if (counter == lvl) {
                    start = i+1;
                } else if (counter == lvl + 1) {
                    end = i;
                    break;
                }
            }
        }

        if (end == 0)
            end = path_len;

        auto name = new char[end - start + 1];
        for (int i = start; i < end; i++)
            name[i - start] = path[i];
        name[end - start] = '\0';

        return name;
    }

    Path *copy() const {
        return new Path(path, path_len);
    }

private:
    void _init(char *str, int str_len = -1, bool rewrite = false) {
        if (rewrite)
            delete[] path;

        if (str_len == -1)
            str_len = (int)strlen(str);

        path_len = str_len;
        path = copy_str(str, str_len);

        if (path_len == 1)
            nesting_lvl = 0;
        else
            nesting_lvl = get_nesting_lvl(path, path_len) - 1;
    }

    void _append(char* path2, int path2_len = -1) {
        if (path2_len == -1)
            path2_len = (int) strlen(path2);

        auto old_path_len = path_len;
        auto old_path = path;

        path_len += path2_len;
        nesting_lvl += get_nesting_lvl(path2, path2_len);

        if (old_path_len != 1)
            path_len++;

        path = new char[path_len + 1];

        int i = 0;
        for (; i < old_path_len; i++)
            path[i] = old_path[i];

        if (old_path_len != 1) {
            path[i] = '/';
            i++;
            old_path_len++;
        }

        for (; i < path_len; i++)
            path[i] = path2[i - old_path_len];
        path[path_len] = '\0';
        delete[] old_path;
    }
};

// =====================================================================

class Obj {
public:
    char *name;
    int name_len;
    Path *path;

    bool is_file;
    int size;

    Obj **children;
    int children_count;
    int children_capacity;

    Obj(
            char *name_arg,
            int name_len_arg = -1,
            Obj *parent_arg = nullptr,
            bool is_file_arg = false,
            int size_arg = 0
    ) {
        if (name_len_arg == -1)
            name_len = (int)strlen(name_arg);
        else
            name_len = name_len_arg;

        name = copy_str(name_arg, name_len);

        if (parent_arg == nullptr) {
            path = new Path(name_arg);
        } else {
            path = new Path(parent_arg->path->path, parent_arg->path->path_len);
            path->cmd_cd(name_arg, name_len_arg);
        }

        is_file = is_file_arg;
        if (is_file_arg) {
            size = size_arg;
            children = nullptr;
            children_count = -1;
            children_capacity = -1;
        } else {
            size = -1;
            children = nullptr;
            children_count = 0;
            children_capacity = 0;
            _expand_children_capacity();
        }
    }

    ~Obj() {
        delete[] name;
        delete path;

        if (is_file)
            return;

        for (int i = 0; i < children_count; i++)
            delete children[i];
        delete[] children;
    }

    int append_child(Obj* obj) {
        if (is_file)
            return 0;
        if (children_count == children_capacity)
            _expand_children_capacity();
        children[children_count] = obj;
        children_count++;
        return 1;
    }

    int remove_child(char *obj_name, int recursive) {
        int obj_name_len = (int)strlen(obj_name);
        int idx = -1;

        for (int i = 0; i < children_count; i++) {
            if (obj_name_len == children[i]->name_len && strncmp(children[i]->name, obj_name, obj_name_len) == 0) {
                idx = i;
                break;
            }
        }

        if (idx == -1)
            return -1;

        if (!(children[idx]->is_file || children[idx]->children_count == 0 || recursive == 1))
            return -2;

        int child_size = _size_of_obj(children[idx]);

        delete children[idx];

        while (idx < children_count - 1) {
            children[idx] = children[idx + 1];
            idx++;
        }
        children_count--;
        return child_size;
    }

    char *get_full_path() const {
        return path->path;
    }

private:
    void _expand_children_capacity() {
        if (is_file)
            return;
        if (children_count == 0) {
            children = new Obj*[CAPACITY_STEP];
            children_capacity = CAPACITY_STEP;
        } else {
            Obj **new_children = new Obj*[children_count + CAPACITY_STEP];
            for (int i = 0; i < children_count; i++) {
                new_children[i] = children[i];
            }
            delete[] children;
            children = new_children;
            children_capacity = children_count + CAPACITY_STEP;
        }
    }

    static int _size_of_obj(Obj *obj) {
        if (obj->is_file)
            return obj->size;

        if (obj->children_count == 0)
            return 0;

        int child_size = 0;
        for (int i = 0; i < obj->children_count; i++)
            child_size += _size_of_obj(obj->children[i]);
        return child_size;
    }
};

// =====================================================================

class MyFileManager {
private:
    Obj root;
    int disk_free_space = 0;
    Obj *working_dir = nullptr;
    bool is_created = false;

    Obj *tmp_obj = nullptr;
    Path *tmp_path = nullptr;

public:
    MyFileManager() : root((char *)"/", 1) {}

    ~MyFileManager() {
        for (int i = 0; i < root.children_count; i++)
            delete root.children[i];
        root.children_count = 0;
        delete tmp_path;
    }

    int init(int init_disk_size) {
        if (init_disk_size < 0 || is_created)
            return 0;
        disk_free_space = init_disk_size;
        working_dir = &root;
        is_created = true;
        return 1;
    }

    int destroy() {
        if (!is_created)
            return 0;
        working_dir = nullptr;
        tmp_obj = nullptr;

        for (int i = 0; i < root.children_count; i++)
            delete root.children[i];
        root.children_count = 0;
        delete tmp_path;
        tmp_path = nullptr;

        disk_free_space = 0;
        is_created = false;
        return 1;
    }

    char *get_working_directory() {
        return working_dir->get_full_path();
    }

    int change_working_directory(const char* path) {
        if (!is_created)
            return 0;

        int path_len = (int)strlen(path);

        delete tmp_path;
        tmp_path = working_dir->path->copy();
        if (_change_tmp_path(path, path_len) != 1)
            return 0;

        if (_cd_tmp_path() != 1)
            return 0;

        if (tmp_obj->is_file)
            return 0;

        working_dir = tmp_obj;
        return 1;
    }

    int create_directory(const char *path) {
        if (!is_created)
            return 0;

        int path_len = (int)strlen(path);
        if (is_special(path, path_len))
            return 0;

        if (_change_tmp_path(path, path_len) != 1)
            return 0;

        auto dir_name = tmp_path->get_parent_name_by_lvl(tmp_path->nesting_lvl);
        int dir_len;

        if (dir_name == nullptr)
            return 0;

        dir_len = (int)strlen(dir_name);
        if (dir_len > 32)
            goto free_and_return_0;


        if (!str_is_valid(dir_name, false))
            goto free_and_return_0;

        if (!tmp_path->up_1_lvl())
            goto free_and_return_0;

        if (_cd_tmp_path() != 1)
            goto free_and_return_0;

        if (_create_dir_in_tmp_path(dir_name) != 1)
            goto free_and_return_0;

        delete[] dir_name;
        return 1;

        free_and_return_0:
        delete[] dir_name;
        return 0;
    }

    int create_file(const char *path, int file_size) {
        if (!is_created)
            return 0;

        if (file_size < 0 || file_size > disk_free_space)
            return 0;

        int path_len = (int)strlen(path);
        if (is_special(path, path_len))
            return 0;

        if (_change_tmp_path(path, path_len) != 1)
            return 0;

        auto file_name = tmp_path->get_parent_name_by_lvl(tmp_path->nesting_lvl);
        int file_len;

        if (file_name == nullptr)
            return 0;

        file_len = (int)strlen(file_name);
        if (file_len > 32)
            goto free_and_return_0;

        if (!str_is_valid(file_name, false))
            goto free_and_return_0;

        if (!tmp_path->up_1_lvl())
            goto free_and_return_0;

        if (_cd_tmp_path() != 1)
            goto free_and_return_0;

        if (_create_file_in_tmp_path(file_name, file_size) != 1)
            goto free_and_return_0;

        delete[] file_name;
        return 1;

        free_and_return_0:
        delete[] file_name;
        return 0;
    }

    int remove(const char *path, int recursive) {
        if (!is_created)
            return 0;

        int path_len = (int)strlen(path);

        if (path_len == 1 && strncmp(path, root.name, 1) == 0)
            return 0;

        if (is_special(path, path_len))
            return 0;

        if (_change_tmp_path(path, path_len) != 1)
            return 0;

        bool working_dir_will_be_removed = strstr(working_dir->path->path, tmp_path->path);

        auto obj_name = tmp_path->get_parent_name_by_lvl(tmp_path->nesting_lvl);
        int obj_size;

        if (obj_name == nullptr)
            return 0;

        if (!tmp_path->up_1_lvl())
            goto free_and_return_0;

        if (_cd_tmp_path() != 1)
            goto free_and_return_0;

        obj_size = tmp_obj->remove_child(obj_name, recursive);

        if (obj_size < 0)
            goto free_and_return_0;

        disk_free_space += obj_size;
        delete[] obj_name;
        if (working_dir_will_be_removed)
            working_dir = &root;
        return 1;

        free_and_return_0:
        delete[] obj_name;
        return 0;
    }

    int free_space() const {
        return disk_free_space;
    }

private:
    int _change_tmp_path(const char *path, int path_len) {
        delete tmp_path;
        tmp_path = working_dir->path->copy();
        if (!tmp_path->cmd_cd((char *)path, path_len))
            return 0;
        return 1;
    }

    int _cd_tmp_path() {
        if (tmp_path == nullptr)
            return -1;

        tmp_obj = &root;
        bool flag = false;

        char *lvl_dir_name;
        int lvl_dir_name_len;
        for (int i = 1; i <= tmp_path->nesting_lvl; i++) {
            lvl_dir_name = tmp_path->get_parent_name_by_lvl(i);
            lvl_dir_name_len = (int)strlen(lvl_dir_name);
            for (int j = 0; j < tmp_obj->children_count; j++) {
                if (tmp_obj->children[j]->name_len == lvl_dir_name_len &&
                strncmp(tmp_obj->children[j]->name, lvl_dir_name, lvl_dir_name_len) == 0) {
                    if (tmp_obj->children[j]->is_file) {
                        delete[] lvl_dir_name;
                        return -1;
                    }
                    tmp_obj = tmp_obj->children[j];
                    flag = true;
                }
            }
            if (!flag) {
                delete[] lvl_dir_name;
                return -2;
            }
            flag = false;
            delete[] lvl_dir_name;
        }
        return 1;
    }

    bool _obj_is_exists_in_tmp_path(char *obj_name, int obj_name_len) const {
        for (int j = 0; j < tmp_obj->children_count; j++) {
            if (
                    obj_name_len == tmp_obj->children[j]->name_len &&
                    strncmp(tmp_obj->children[j]->name, obj_name, obj_name_len) == 0
                    ) {
                return true;
            }
        }
        return false;
    }

    int _create_dir_in_tmp_path(char *dir_name) {
        int new_dir_name_len = (int)strlen(dir_name);

        if (_obj_is_exists_in_tmp_path(dir_name, new_dir_name_len))
            return -1;

        if (tmp_obj->append_child(new Obj(dir_name, new_dir_name_len, tmp_obj)) != 1)
            return -2;
        return 1;
    }

    int _create_file_in_tmp_path(char *file_name, int file_size) {
        int new_file_name_len = (int)strlen(file_name);

        if (_obj_is_exists_in_tmp_path(file_name, new_file_name_len))
            return -1;

        if (tmp_obj->append_child(new Obj(file_name, new_file_name_len, tmp_obj, true, file_size)) != 1)
            return -2;

        disk_free_space -= file_size;
        return 1;
    }
};

// =====================================================================

static MyFileManager mfm;

void setup_file_manager(file_manager_t *fm) {
    fm->create = [](int disk_size) {return mfm.init(disk_size);};
    fm->destroy = []() {return mfm.destroy();};
    fm->create_dir = [](const char* path) {return mfm.create_directory(path);};
    fm->create_file = [](const char *path, int file_size) {return mfm.create_file(path, file_size);};
    fm->remove = [](const char *path, int recursive) {return mfm.remove(path, recursive);};
    fm->change_dir = [](const char* path) {return mfm.change_working_directory(path);};
    fm->get_cur_dir = [](char* dst) {strcpy(dst, mfm.get_working_directory());};
    fm->free_space = []() {return mfm.free_space();};
}
