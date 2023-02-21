#include <vfs.h>
#include <string.h>
#include <heap.h>
#include <vga.h>
#include <errorcodes.h>
#include <memory.h>
#include <ustar.h>

namespace VFS_TREE {
    VFS::vfs_tree_t* create_tree(void) {
        VFS::vfs_tree_t *tree = new VFS::vfs_tree_t;
        tree->nodes = 0;
        tree->root = 0;
        return tree;
    }
    VFS::vfs_tree_node_t* create_node(void) {
        VFS::vfs_tree_node_t *node = new VFS::vfs_tree_node_t;
        node->childs = 0;
        node->next = 0;
        node->prev = 0;
        node->children = 0;
        node->value = 0;
        return node;
    }
    void set_value(VFS::vfs_tree_node_t* node, VFS::vfs_entry_t* value) {
        node->value = value;
    }
    void set_root(VFS::vfs_tree_t* tree, VFS::vfs_tree_node_t* root) {
        tree->root = root;
    }
    void insert_child(VFS::vfs_tree_t* tree, VFS::vfs_tree_node_t* parent, VFS::vfs_tree_node_t* child) {
        if (parent->children == NULL || parent->childs == 0) parent->children = child;
        else {
            VFS::vfs_tree_node_t* current = parent->children;
            while (current->next) current = current->next;
            current->next = child;
        }
        tree->nodes++;
        parent->childs++;
    }
    VFS::vfs_tree_node_t* get_child(VFS::vfs_tree_node_t* parent, char* name) {
        if (!parent) return 0;
        if (!name) return 0;
        VFS::vfs_tree_node_t* current = parent->children;
        while (current) {
            if (!current->value) { current = current->next; continue; }
            if (strcmp(current->value->name,name)) return current;
            current = current->next;
        }
        return current;
    }
}

namespace VFS {
    vfs_tree_t* vfs_root;
    void install_vfs(void) {
        vfs_root = VFS_TREE::create_tree();
        vfs_tree_node_t *root_node = VFS_TREE::create_node();
        vfs_entry_t *root = new vfs_entry_t;
        root->name = strdup("[root]");
        root->file = NULL;
        root->device = NULL;
        root->fs_type = NULL;
        VFS_TREE::set_value(root_node,root);
        VFS_TREE::set_root(vfs_root,root_node);
    }
    void print_vfs_tree_node(vfs_tree_node_t *node) {
        if (!node->value) return;
        printf("%s%s\n\r",node->value->name,node->childs ? "{" : ";");
        if (!node->childs) return;
        vfs_tree_node_t* current = node->children;
        while (current) {
            print_vfs_tree_node(current);
            current = current->next;
        }
        print("}\n\r");
    }
    void print_vfs(void) {
        print_vfs_tree_node(vfs_root->root);
    }
    int vfs_get_tree_node(char* ipath,vfs_tree_node_t* out) {
        char* path = strdup(ipath);
        char* org_path = path;
        if (path[0] != PATH_SEPERATOR) return ERELATIVE;
        vfs_tree_node_t* current = vfs_root->root;
        uint64_t path_len = strlen(path);
        for (uint64_t i = 0; i < path_len; i++) {
            if (path[i] == PATH_SEPERATOR) path[i] = 0;
        }
        path++;
        while (path_len--) {
            vfs_tree_node_t* new_current = VFS_TREE::get_child(current,path);
            if (!new_current) {
                printf("[VFS] Create node %s\n\r", path);
                new_current = VFS_TREE::create_node();
                new_current->value = new vfs_entry_t;
                new_current->value->name = strdup(path);
                VFS_TREE::insert_child(vfs_root,current,new_current);
            }
            current = new_current;
            path_len -= strlen(path);
            path += strlen(path) + 1;
        }
        heap::free(org_path);
        *out = *current;
        return SUCCESS;
    }
    int vfs_get_file(char* path,vfs_entry_t* out) {
        vfs_tree_node_t* node;
        int ret = vfs_get_tree_node(path,node);
        if (ret) return ret;
        out = node->value;
        return SUCCESS;
    }
    vfs_entry_t* vfs_create_mountpoint(char* name,uint64_t impl) {
        vfs_entry_t* entry = new vfs_entry_t;
        entry->device = 0;
        switch (impl) {
            case FS_IMPL_USTAR: entry->fs_type = strdup("USTAR"); break;
            default: entry->fs_type = NULL;
        }
        entry->name = strdup(name);
        fs_node_t *file = new fs_node_t;
        file->impl = impl;
        memcpy(file->name,entry->name,strlen(entry->name) < 256 ? strlen(entry->name) : 256);
        file->type = FS_MOUNTPOINT;
        entry->file = file;
        return entry;
    }
    void vfs_create_file(char* name, vfs_tree_node_t* parent, uint64_t size, uint8_t type, uint64_t mtime, uint64_t position=0) {
        fs_node_t* file = new fs_node_t;
        memcpy(file->name,name,strlen(name) < 256 ? strlen(name) : 256);
        file->mtime = mtime;
        file->atime = mtime;
        file->ctime = mtime;
        file->type = type;
        file->size = size;
        file->position = position;
        vfs_entry_t* entry = new vfs_entry_t;
        entry->device = 0;
        entry->fs_type = 0;
        entry->name = strdup(name);
        entry->file = file;
        vfs_tree_node_t* node = VFS_TREE::create_node();
        VFS_TREE::set_value(node,entry);
        VFS_TREE::insert_child(vfs_root,parent,node);
    } 
    void vfs_mount_ustar(USTAR::ustar_t* tar, vfs_tree_node_t* to,char* name) {
        VFS_TREE::set_value(to,vfs_create_mountpoint(name,FS_IMPL_USTAR));
        for (uint64_t i = 0; i < MAX_FILES; i++) {
            USTAR::file_t* file = &tar->files[i];
            if (file->entry.name[0] == '\0') break;
            vfs_create_file(strdup(file->entry.name),to,oct2bin((uint8_t*)file->entry.size,11),FS_FILE,oct2bin((uint8_t*)file->entry.mtime,11),file->lba);
        }
    }
    int vfs_mount(char* path,void* mount,uint64_t impl) {
        vfs_tree_node_t* node;
        uint64_t ret = vfs_get_tree_node(path,node);
        if (ret) return ret;
        if (!node->value) return EINVAL;
        if (node->value->file) return EUSED;
        switch (impl) {
            case FS_IMPL_USTAR: vfs_mount_ustar((USTAR::ustar_t*)mount,node,"ustar"); break;
            default: printf("[VFS] Error FS_IMPL %h not supported\n\r",impl); return EINVAL;
        }
        return SUCCESS;
    }

    int add_device(void* dev, char* name) {
        print("[VFS] Adding device at ");
        print_hex((uint64_t)dev);
        print(" as ");
        print(name);
        new_line();
        char* path = name;
        if (name[0] != PATH_SEPERATOR) path = strcat("/",name);
        vfs_tree_node_t* node;
        int ecode = vfs_get_tree_node(path,node);
        if (ecode) return ecode;
        vfs_entry_t* entry = node->value;
        if (!entry) { print("[VFS] Node doesn't have value creating entry\n\r"); entry = node->value = new vfs_entry_t; }
        entry->device_ptr = dev;
        return SUCCESS;
    }
}