#include <iostream>
#include <vector>
#include <memory>

using std::shared_ptr, std::make_shared;

namespace tree {
    template <typename K>
    struct Node {
        K                   data;
        size_t              count;
        shared_ptr<Node<K>> left;
        shared_ptr<Node<K>> right;

        Node(K data_) : data(data_), count(1), left(nullptr), right(nullptr) {} 
    };

    template <typename K>
    struct NodeWithDepth {
        const Node<K>*  node;
        const size_t    depth;
    };

    template <typename K>
    std::ostream& operator<<(std::ostream& out, const Node<K> &node) {
        return out << NodeWithDepth<K> { &node, 0 };
    }

    template <typename K>
    std::ostream& operator<<(std::ostream& out, const NodeWithDepth<K> &noded) {
        const Node<K>* node = noded.node;
        if (! node) return out;

        for (size_t i = 0; i < noded.depth; i ++) out << "   ";

        out << node->data;
        if (node->count > 1) out << " (* " << node->count << ")";
        out << std::endl;

        out << NodeWithDepth<K> { node->left.get(), noded.depth + 1 };
        out << NodeWithDepth<K> { node->right.get(), noded.depth + 1 };

        return out;
    }

    namespace bst {
        template <typename K>
        void insert(shared_ptr<Node<K>> &tree, K data) {
            if (! tree)
                tree = make_shared<Node<K>>(data);
            else if (data == tree->data)
                tree->count ++;
            else if (data < tree->data)
                insert(tree->left, data);
            else
                insert(tree->right, data);
        }

        template <typename K>
        shared_ptr<Node<K>> find(shared_ptr<Node<K>> tree, K data) {
            if (! tree)
                return nullptr;
            if (data < tree->data)
                return find(tree->left, data);
            if (data > tree->data)
                return find(tree->right, data);
            return tree;
        }

        template <typename K>
        shared_ptr<Node<K>> find_min(shared_ptr<Node<K>> tree) {
            if (! tree)
                return nullptr;
            if (! tree->left)
                return tree;
            return find_min(tree->left);
        }

        template <typename K>
        shared_ptr<Node<K>> find_max(shared_ptr<Node<K>> tree) {
            if (! tree)
                return nullptr;
            if (! tree->right)
                return tree;
            return find_max(tree->right);
        }

        template <typename K>
        shared_ptr<Node<K>> remove(shared_ptr<Node<K>> &tree, K data) {
            if (! tree)
                return nullptr;

            if (data < tree->data)
                return remove(tree->left, data);
            if (data > tree->data)
                return remove(tree->right, data);

            if (tree->count > 1) {
                tree->count --;
                return tree;
            }

            auto removed = tree;

            if (tree->left && tree->right) {
                auto succ = find_min(tree->right);
                tree = succ;
                tree->left = removed->left;
                remove(tree->right, succ->data);
                removed->left = removed->right = nullptr;
            }

            else if (tree->left) {
                tree = tree->left;
            }
            else if (tree->right) {
                tree = tree->right;
            }
            else {
                tree = nullptr;
            }

            return removed;
        }
    }
}

struct Point {
    int x;
    int y;

    auto operator<=>(const Point& that) const {
        return x == that.x ? y <=> that.y : x <=> that.x;
    }
    bool operator==(const Point& that) const = default;
};

std::ostream& operator<<(std::ostream& out, const Point &p) {
    return out << "(" << p.x << ", " << p.y << ")";
}

int main() {
    std::vector<Point> es {
        { 2, 3 },
        { 3, 4 },
        { 1, 2 },
        { 4, 5 },
    };
    shared_ptr<tree::Node<Point>> t { nullptr };
    for (auto e : es) {
        tree::bst::insert(t, e);
    }
    std::cout
        << "t0:\n" << *t
        << "min: " << *tree::bst::find_min(t)
        << "max: " << *tree::bst::find_max(t)
    ;
    auto removed = tree::bst::remove(t, { 2, 3 });
    std::cout
        << "removed:\n" << *removed
        << "t1:\n" << *t
    ;
    return 0;
}