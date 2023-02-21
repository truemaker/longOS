#pragma once
#include <typedef.h>

#define PATH_SEPERATOR '/'
#define PATH_SEPERATOR_STRING "/"
#define PATH_UP ".."
#define PATH_DOT "."

#define FS_FILE         0x01
#define FS_DIR          0x02
#define FS_CHARDEV      0x04
#define FS_BLOCKDEV     0x08
#define FS_PIPE         0x10
#define FS_SYMLINK      0x20
#define FS_MOUNTPOINT   0x40

#define FS_IMPL_USTAR   0x01

namespace VFS {
    struct fs_node;

    typedef size_t (*read_type_t) (struct fs_node *, uint64_t, size_t, uint8_t *);
    typedef size_t (*write_type_t) (struct fs_node *, uint64_t, size_t, uint8_t *);
    typedef void (*open_type_t) (struct fs_node *, uint32_t flags);
    typedef void (*close_type_t) (struct fs_node *);

    typedef struct fs_node {
        char name[256];
        void* device;           // If available
        uint64_t impl;          // Used FS
        uint8_t type;

        // The following are only used if available
        uint64_t position;
        uint64_t size;
        uint64_t atime;
        uint64_t mtime;
        uint64_t ctime;

        // Required functions for interaction with file
        read_type_t read;
        write_type_t write;
        open_type_t open;
        close_type_t close;

        struct fs_node* link; // Only for symlinks

        uint64_t references;
    } fs_node_t;

    typedef struct vfs_entry {
        char* name;
        fs_node_t* file;
        char* device;
        void* device_ptr;
        char* fs_type;
    } vfs_entry_t;

    typedef struct vfs_tree_node {
        vfs_entry_t* value;
        struct vfs_tree_node* prev;
        struct vfs_tree_node* next;
        struct vfs_tree_node* children;
        uint64_t childs;
    } vfs_tree_node_t;

    typedef struct vfs_tree {
        vfs_tree_node_t* root;
        uint64_t nodes;
    } vfs_tree_t;

    extern vfs_tree_t* vfs_root;

    void install_vfs(void);
    void print_vfs(void);
    int vfs_mount(char*,void*,uint64_t);
    int add_device(void* dev, char* name);
}