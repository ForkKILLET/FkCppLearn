#include <cassert>
#include <cstddef>
#include <functional>
#include <memory>
#include <iostream>
#include <ostream>
#include <vector>
#include <format>

enum class Direction : short {
    ROOT = 0,
    LEFT = -1,
    RIGHT = 1,
};

Direction operator-(Direction dir) {
    return static_cast<Direction>(- static_cast<short>(dir));
}

template <typename K, typename V>
struct TreeMap {
private:
    enum class Color : bool {
        RED,
        BLACK,
    };

    struct Node {
        using PNode = std::shared_ptr<Node>;

        K           key;
        V           value;
        Color       color;
        PNode       parent;
        PNode       left;
        PNode       right;

        explicit Node(K key, V value) :
            key(std::move(key)),
            value(std::move(value)),
            color(Color::RED),
            parent(nullptr),
            left(nullptr),
            right(nullptr) {}
        
        static PNode from(const K& key, const V& value) {
            return std::make_shared<Node>(key, value);
        }

        bool isRed() const {
            return color == Color::RED;
        }
        bool isBlack() const {
            return color == Color::BLACK;
        }

        bool isLeaf() const {
            return ! left && ! right;
        }

        PNode onlyChild() const {
            return left ? left : right;
        }

        Direction direction() const {
            if (! parent)
                return Direction::ROOT;
            return this == parent->left.get()
                ? Direction::LEFT
                : Direction::RIGHT;
        }

        PNode sibling() const {
            return direction() == Direction::LEFT
                ? parent->right
                : parent->left;
        }

        PNode min() const {
            PNode node = left;
            while (node->left)
                node = node->left;
            return node;
        }
        PNode max() const {
            PNode node = right;
            while (node->right)
                node = node->right;
            return node;
        }

        PNode prev() const {
            assert(left);
            return left->max();
        }
        PNode next() const {
            assert(right);
            return right->min();
        }
    };

    using PNode = typename Node::PNode;

    PNode root;
    size_t _size;

    void _replace_node(const PNode& old, PNode rep) {
        switch (old->direction()) {
            case Direction::LEFT:
                old->parent->left = rep;
                break;
            case Direction::RIGHT:
                old->parent->right = rep;
                break;
            case Direction::ROOT:
                root = rep;
                break;
        }
        if (rep) rep->parent = old->parent;
    }

    void _rotate_left(const PNode& node) {
        PNode rep = node->right;
        _replace_node(node, rep);
        node->parent = rep;
        node->right = rep->left;
        if (node->right) node->right->parent = node;
        rep->left = node;
    }

    void _rotate_right(const PNode& node) {
        PNode rep = node->left;
        _replace_node(node, rep);
        node->parent = rep;
        node->left = rep->right;
        if (node->left) node->left->parent = node;
        rep->right = node;
    }

    void _rotate(const PNode& node, Direction dir) {
        if (dir == Direction::LEFT)
            _rotate_left(node);
        else // (dir == Direction::RIGHT)
            _rotate_right(node);
    }

    template<typename R>
    R _get(const K& key, const PNode& node, std::function<R(PNode)> on_found, std::function<R()> on_not_found) const {
        if (! node)
            return on_not_found();
        if (key < node->key)
            return _get<R>(key, node->left, on_found, on_not_found);
        if (key > node->key)
            return _get<R>(key, node->right, on_found, on_not_found);
        return on_found(node);
    }

    V& _get_or_insert(const K& key, std::function<V()> func, PNode& node, const PNode& parent) {
        if (! node) {
            node = Node::from(key, func());
            node->parent = parent;
            _insert(node);
            return node->value;
        }
        if (key < node->key)
            return _get_or_insert(key, func, node->left, node);
        if (key > node->key)
            return _get_or_insert(key, func, node->right, node);
        return node->value;
    }

    void _insert(const PNode& node) {
        ++ _size;
        _maintain_after_insert(node);
    }

    void _maintain_after_insert(const PNode& node) {
        // Case 1: Empty tree
        // Case 2: Parent is black
        if (! node->parent || node->parent->isBlack())
            return;

        // Case 3: Parent is red and parent is root
        if (node->parent == root) {
            node->parent->color = Color::BLACK;
            return;
        }

        PNode parent = node->parent;
        PNode grandparent = parent->parent;
        PNode uncle = parent->sibling();

        // Case 4: Parent and uncle are red
        if (uncle && uncle->isRed()) {
            parent->color = Color::BLACK;
            uncle->color = Color::BLACK;
            grandparent->color = Color::RED;
            _maintain_after_insert(grandparent);
            return;
        }

        // Case 5: Parent is red and uncle is black

        // Case 5.1: Node has different direction with parent
        Direction parentDir = parent->direction();
        if (node->direction() != parentDir)
            _rotate(parent, parentDir);

        // Case 5.2: Node has same direction with parent
        _rotate(grandparent, - parentDir);
        parent->color = Color::BLACK;
        grandparent->color = Color::RED;
    }

    void _remove_node(PNode node) {
        assert(node);

        // Case 1: Node is the only one in the tree
        if (_size == 1) {
            root = nullptr;
            return;
        }

        // Case 2: Node has two children
        if (node->left && node->right) {
            PNode prev = node->prev();
            node->key = prev->key;
            node->value = prev->value;

            node = prev;
        }

        // Case 3: Node has only one child,
        //         so the child must be red,
        //         and node itself must be black
        PNode child = node->onlyChild();
        if (child) {
            _replace_node(node, child);
            child->color = Color::BLACK;
            return;
        }

        // Case 4: Node has no child

        // Case 4.1: Node is black
        if (node->isBlack()) _maintain_after_remove(node);

        _replace_node(node, nullptr);
    }

    void _maintain_after_remove(const PNode& node) {
        assert(node->isBlack() && node->isLeaf());

        PNode sibling = node->sibling();
        PNode parent = node->parent;

        // Case 1: Sibling is red
        if (sibling->isRed()) {
            _rotate(parent, node->direction());
            sibling->color = Color::BLACK;
            parent->color = Color::RED;

            sibling = node->sibling();
        }

        PNode closeNephew = node->direction() == Direction::LEFT
            ? sibling->left
            : sibling->right;
        PNode distantNephew = node->direction() == Direction::LEFT
            ? sibling->right
            : sibling->left;

        bool closeNephewIsBlack = ! closeNephew || closeNephew->isBlack();
        bool distantNephewIsBlack = ! distantNephew || distantNephew->isBlack();

        if (closeNephewIsBlack && distantNephewIsBlack) {
            // Case 2: Both nephews are black and parent is red
            if (parent->isRed()) {
                parent->color = Color::BLACK;
                sibling->color = Color::RED;
                return;
            }

            // Case 3: Both nephews are black and parent is black
            sibling->color = Color::RED;
            _maintain_after_remove(parent);
            return;
        }

        // Case 4: Close nephew is red
        if (! closeNephewIsBlack) {
            _rotate(sibling, sibling->direction());
            closeNephew->color = Color::BLACK;
            sibling->color = Color::RED;
            distantNephew = sibling;
            sibling = closeNephew;
        }

        // Case 5: Distant nephew is red
        _rotate(parent, node->direction());
        sibling->color = parent->color;
        parent->color = Color::BLACK;
        distantNephew->color = Color::BLACK;
    }

    void _print_node(std::ostream& out, const PNode& node, int depth) const {
        for (int i = 0; i < depth; ++ i) out << "    ";
        if (! node) {
            out << "\x1B[30m∅\x1B[0m" << std::endl;
            return;
        }
        out << (node->isRed() ? "\x1B[31m" : "\x1B[30m") << node->key << "\x1B[0m" << std::endl;
        _print_node(out, node->left, depth + 1);
        _print_node(out, node->right, depth + 1);
    }

public:
    TreeMap() :
        root(nullptr),
        _size(0) {}

    const size_t& size = _size;

    bool empty() const {
        return _size == 0;
    }

    const V& get_or(const K& key, std::function<V&()> on_not_found) const {
        return _get<V&>(key, root, [](PNode node) -> V& {
            return node->value;
        }, on_not_found);
    }

    const V& get(const K& key) const {
        return get_or(key, [&key] -> V& {
            throw std::out_of_range(std::format("Key '{}' not found", key));
        });
    }

    const V& get_or_else(const K& key, const V& def) const {
        return get_or(key, [&def]() {
            return def;
        });
    }

    V& get_or_insert(const K& key, std::function<V()> func) {
        return _get_or_insert(key, func, root, nullptr);
    }

    V& set(const K& key, const V& value) {
        return get_or_insert(key, [&value] {
            return value;
        });
    }

    V operator[](const K& key) const {
        return get(key);
    }

    V& operator[](const K& key) {
        return get_or_insert(key, [] {
            return V();
        });
    }

    bool remove(const K& key) {
        return _get<bool>(key, root, [this](PNode node) {
            _remove_node(node);
            -- _size;
            return true;
        }, [] {
            return false;
        });
    }

    void print(std::ostream &out = std::cout) const {
        _print_node(out, root, 0);
    }
};

template <typename K, typename V>
std::ostream& operator<<(std::ostream& out, const TreeMap<K, V>& tree) {
    tree.print(out);
    return out;
}

int main() {
    std::vector<int> keys = { 1, 2, 3, 4, 8 ,7, 6, 5 };
    TreeMap<int, int> tree;

    for (int key : keys) {
        tree[key] = key * key;
        tree.print();
    }
    
    tree.remove(8);
    tree.print();

    std::cout << "7 * 7 = " << tree[7] << std::endl;
    std::cout << "9 * 9 = " << tree.get(9) << std::endl;

    return 0;
}