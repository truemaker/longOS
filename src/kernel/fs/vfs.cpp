#include <vfs.h>
#include <string.h>
#include <heap.h>
#include <vga.h>

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
        printf("%s%s\n\r",node->value->name,node->children ? " {" : ";");
        if (!node->children) return;
        vfs_tree_node_t* current = node->children;
        while (current->next) {
            print_vfs_tree_node(current);
            current = current->next;
        }
        print("}\n\r");
    }
    void print_vfs(void) {
        print_vfs_tree_node(vfs_root->root);
    }
}