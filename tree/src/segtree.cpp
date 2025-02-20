#include <cmath>
#include <cstddef>
#include <ostream>
#include <vector>
#include <iostream>

template<typename T>
struct SegTree {
private:
    struct Seg {
        T sum;
        T tag;
    };

    std::vector<Seg> segs;
    size_t _size;

    T _build(const T base[], size_t start, size_t end, size_t id) {
        T sum;
        if (start == end) {
            sum = base[start];
        }
        else {
            size_t mid = (start + end) / 2;
            sum = _build(base, start, mid, id * 2)
                + _build(base, mid + 1, end, id * 2 + 1);
        }
        segs[id] = { .sum = sum, .tag = 0 };
        return sum;
    }

    inline void _push_down(Seg& seg, Seg& left, Seg& right, size_t start, size_t end, size_t mid) {
        left.tag += seg.tag;
        left.sum += seg.tag * (mid - start + 1);
        right.tag += seg.tag;
        right.sum += seg.tag * (end - mid);
        seg.tag = 0;
    }

    T _query(size_t qstart, size_t qend, size_t start, size_t end, size_t id) {
        Seg& seg = segs[id];
        if (qstart <= start && end <= qend) {
            return seg.sum;
        }

        size_t mid = (start + end) / 2;

        Seg& left = segs[id * 2];
        Seg& right = segs[id * 2 + 1];
        if (seg.tag) _push_down(seg, left, right, start, end, mid);

        T sum = 0;
        if (qstart <= mid) {
            sum += _query(qstart, qend, start, mid, id * 2);
        }
        if (qend > mid) {
            sum += _query(qstart, qend, mid + 1, end, id * 2 + 1);
        }
        return sum;
    }

    void _seg_update(size_t qstart, size_t qend, T inc, size_t start, size_t end, size_t id) {
        Seg& seg = segs[id];
        if (qstart <= start && end <= qend) {
            seg.tag += inc;
            seg.sum += inc * (end - start + 1);
            return;
        }

        size_t mid = (start + end) / 2;

        Seg& left = segs[id * 2];
        Seg& right = segs[id * 2 + 1];
        if (seg.tag) _push_down(seg, left, right, start, end, mid);

        if (qstart <= mid) {
            _seg_update(qstart, qend, inc, start, mid, id * 2);
        }
        if (qend > mid) {
            _seg_update(qstart, qend, inc, mid + 1, end, id * 2 + 1);
        }
        seg.sum = left.sum + right.sum;
    }
public:
    explicit SegTree(const T base[], size_t size) :
        _size(size)
    {
        segs.resize(size * 4);
        _build(base, 0, size - 1, 1);
    }

    explicit SegTree(const std::vector<T>& base) :
        SegTree(base.data(), base.size()) {}

    T query(size_t qstart, size_t qend) {
        return _query(qstart, qend, 0, _size - 1, 1);
    }

    void seg_update(size_t qstart, size_t qend, T inc) {
        _seg_update(qstart, qend, inc, 0, _size - 1, 1);
    }

    const size_t& size = _size;
};

void unsync_ios() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
}

int main() {
    unsync_ios();
    size_t n, q;
    std::cin >> n >> q;
    std::vector<long long> base(n);
    for (size_t i = 0; i < n; ++ i) {
        std::cin >> base[i];
    }
    SegTree<long long> segtree(base);
    size_t op, l, r;
    long long x;
    for (size_t i = 0; i < q; ++ i) {
        std::cin >> op >> l >> r;
        if (op == 1) {
            std::cin >> x;
            segtree.seg_update(l - 1, r - 1, x);
        }
        else {
            std::cout << segtree.query(l - 1, r - 1) << std::endl;
        }
    }
    return 0;
}