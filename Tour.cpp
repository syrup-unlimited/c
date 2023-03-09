#include <iostream>
#include <variant>
#include <string>
#include <set>
#include <vector>
#include <unordered_set>
#include <memory>
#include <concepts>
#include <vector>
#include <execution>
#include <random>

namespace color_space {
    // C enums, global scoped names.
    enum colors { RED, GREEN, BLUE };
}

namespace month_space {
    // Cpp enums, class scoped objects.
    enum class months { JAN, FEB, DEC };
    std::ostream& operator<<(std::ostream& _os,const months& _month) {
        _os << static_cast<int>(_month);
        return _os;
    }
}

namespace basic {

class IntP {
    int* p;

public:
    IntP() :p(new int(0)) {
        std::cout << "Default constructor is called" << std::endl;
    }
    ~IntP() {
        std::cout << "Destructor is called" << std::endl;
        delete p;
    }
    IntP(const IntP& _int) {
        std::cout << "Copy constructor is called" << std::endl;
        p = new int(*_int.p);
    }
    IntP& operator=(const IntP& _int) {
        std::cout << "Copy assignment is called" << std::endl;
        delete p;
        p = new int(*_int.p);
        return *this;
    }
    IntP(IntP&& _int) noexcept{
        std::cout << "Move constructor is called" << std::endl;
        p = _int.p;
        _int.p = nullptr;
    }
    IntP& operator=(IntP&& _int) noexcept{
        std::cout << "Move assignment is called" << std::endl;
        delete p;
        p = _int.p;
        _int.p = nullptr;
        return *this;
    }

    int getV() const { return *p; }
    void setV(int _v) { *p = _v; }

    class Compare {
    public:
        // function call operator
        bool operator() (const IntP& _v1, const IntP& _v2) const {
            return _v1.getV() < _v2.getV();
        }
    };
};

IntP createIntP() { IntP v; v.setV(666); return v; }
void updateIntP(IntP& _v) { _v.setV(888); }

template<class T, class X, class Y> using uset = std::unordered_set<T, X, Y>;

void testIntP()
{
    using std::vector;
    using std::set;
    using std::unordered_set;

    IntP v1, v3;
    IntP v2(createIntP()); // move construct
    v1 = v2; // copy assignment
    v2 = std::move(v1); // move assignment
    updateIntP(v3);

    vector<IntP> vp1, vp2;
    vp1.reserve(2); // without, move construct
    vp1.push_back(v2); // copy construct
    vp1.push_back(v3);
    vp2 = vp1;

    set <IntP, IntP::Compare> sp;
    sp.insert(v2);
    sp.insert(v3);
    // how is the comparison done?
    set <IntP> sp_wo_cmp;
    // won't compare if comment below is removed
    // only use operator< when insert
    /* sp_wo_cmp.insert(v2); */

    // why operator== is explicitly delete?
    auto cmp = [](const IntP& _v)->size_t { return _v.getV(); };
    auto eq = [](const IntP& _v1, const IntP& _v2)->bool { return _v1.getV() == _v2.getV(); };
    uset<IntP, decltype(cmp), decltype(eq)> usp;
    usp.insert(v2);
    usp.insert(v3);

    // crash if comment below is removed
    // memory leak
    /*
      IntP* pp1 = new IntP[2];
      delete pp1;
    */

    IntP* pp2 = new IntP[2];
    delete [] pp2;
}

}

namespace templ {

template<typename Iter, typename Val>
Val my_accumulate(Iter first, Iter last, Val res)
{
    for (auto p = first; p != last; ++p)
        res += *p;
    return res;
}

// variadic
template<typename T, typename ... Tail>
void print(T head, Tail... tail)
{
    std::cout << head << " ";
    if constexpr(sizeof...(tail) > 0)
        print(tail...);
}

// fold expressions
template<typename ... T>
int sum(T... v)
{
    return (v + ... + 0);
}

template<typename ...T>
void printFold(T&&... args)
{
    (std::cout << ... << args) << '\n';
}

// forward arguments
void overloaded(const int& x) { std::cout << "[lvalue]\n"; }
void overloaded(int&& x) { std::cout << "[rvalue]\n"; }

template <class T> void fn(T&& x) {
    overloaded(x);
    overloaded(std::forward<T>(x)); // forward rvalue
}

void doAccumulate()
{
    using namespace std;
    vector<int> vec{1, 2, 3};
    list<int> lst{ 4, 5, 6 };
    auto sum1 = my_accumulate(begin(vec), end(vec), 0);
    auto sum2 = my_accumulate(begin(lst), end(lst), 0.0);

    print("Result is:", sum1, "and", sum2);
    cout << endl;
    print("Sum is:", sum(7, 8, 9));
    cout << endl;
    printFold("Hello", ' ', "from ", 2023);
    cout << endl;

    int a; fn(a); fn(1);
}

}

void enumAndLiterals()
{
    using std::cout;
    using std::endl;
    using std::variant;

    variant<color_space::colors, month_space::months> v = color_space::RED;
    // variant is not compatible with ternary operator
    if (holds_alternative<color_space::colors>(v))
        cout << std::get<color_space::colors>(v);
    else
        cout << std::get<month_space::months>(v);
    cout << endl;

    // user defined literal & (raw) string literal
    using namespace std::string_literals;
    cout << "a string"s.append(R"raw( appened with "another" string.)raw") << endl;
}

namespace Alg
{
template<typename C, typename V>
std::vector<typename C::iterator> find_all(C &c, V v)
{
    std::vector<typename C::iterator> res;
    for (auto p = c.begin(); p != c.end(); p++)
    {
        if (*p == v)
        {
            res.push_back(p);
        }
    }
    return res;
}

void func()
{
    using namespace std;

    vector<int> ints{1,2,3};
    // vector<int> copied1{ 3 }; // 
    vector<int> copied1(3);
    vector<int> copied2, copied3;
    copy(ints.begin(), ints.end(), copied1.begin());
    copy(ints.begin(), ints.end(), back_inserter(copied2));
    // copy(ints.begin(), ints.end(), front_inserter(copied3)); // does not compile

    auto find_res = find_all(ints, 2);
    
    auto measure = [](auto policy, vector<uint64_t> v) {
        const auto start = std::chrono::steady_clock::now();
        sort(policy, v.begin(), v.end());
        const auto finish = std::chrono::steady_clock::now();
        cout << chrono::duration_cast<chrono::milliseconds>(finish - start).count() << "\n";
    };

    vector<uint64_t> v(1'000'000);
    mt19937 gen{ random_device{}() };
    ranges::generate(v, gen);

    measure(execution::seq, v);
    measure(execution::unseq, v);
    measure(execution::par_unseq, v);
    measure(execution::par, v);
}

}

int main()
{
    enumAndLiterals();

    basic::testIntP();

    templ::doAccumulate();

    Alg::func();

    return 0;
}
