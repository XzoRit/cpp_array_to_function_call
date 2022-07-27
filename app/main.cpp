#include <any>
#include <array>
#include <iostream>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace xzr
{
using params = std::vector<std::any>;

template <class... Args, std::size_t... I>
void apply(void (*func)(Args...), const params& ps, std::index_sequence<I...>)
{
    func(std::any_cast<std::decay_t<Args>>(ps.at(I))...);
}

template <class... Args>
void apply(void (*func)(Args...), const params& ps)
{
    apply(func, ps, std::make_index_sequence<sizeof...(Args)>{});
}

struct handler
{
    template <class... Args>
    handler(void (*func)(Args... args))
        : erased_func{reinterpret_cast<void (*)()>(func)}
        , dispatch_func{[](void (*erased)(), const params& ps) {
            if (ps.size() != sizeof...(Args))
                throw std::invalid_argument{"size missmatch!"};

            auto actual_func{reinterpret_cast<void (*)(Args...)>(erased)};

            if constexpr (sizeof...(Args) == 0)
            {
                actual_func();
            }
            else
            {
                apply(actual_func, ps);
            }
        }}
    {
    }

    void operator()(const params& ps) const
    {
        dispatch_func(erased_func, ps);
    }

  private:
    void (*erased_func)(){nullptr};
    void (*dispatch_func)(void (*)(), const params&){nullptr};
};
}

void void_func()
{
    std::cout << __func__ << '\n';
}

void int_int(int a, int b)
{
    std::cout << __func__ << '(' << a << ", " << b << ')' << '\n';
}

void std_string_view(std::string_view a)
{
    std::cout << __func__ << "(\"" << a << "\")" << '\n';
}

void int_const_ref_string(int a, const std::string& b)
{
    std::cout << __func__ << "(" << a
              << ", "
                 "\""
              << b << "\")" << '\n';
}

int main()
{
    using namespace std::literals::string_view_literals;
    using namespace std::string_literals;

    {
        using xzr::handler;
        using xzr::params;

        {
            const params ps{};
            const handler h{void_func};

            h(ps);
        }
        {
            const params ps{1, 22};
            const handler h{int_int};

            h(ps);
        }
        {
            const params ps{"string_view"sv};
            const handler h{std_string_view};

            h(ps);
        }
        {
            const params ps{1, "string"s};
            const handler h{int_const_ref_string};

            h(ps);
        }
        {
            const params ps{111};
            const handler h{std_string_view};

            try
            {
                h(ps);
            }
            catch (const std::exception& e)
            {
                std::cout << e.what() << '\n';
            }
        }
        {
            const params ps{1, 22, 333};
            const handler h{int_int};

            try
            {
                h(ps);
            }
            catch (const std::exception& e)
            {
                std::cout << e.what() << '\n';
            }
        }
        {
            std::cout << "\n";
            using id_handler = std::tuple<std::string_view, handler>;
            using id_params = std::tuple<std::string_view, params>;

            const auto handlers{std::array<id_handler, 2>{
                {{"1", int_int}, {"2", std_string_view}}}};
            const auto parameters{std::array<id_params, 2>{
                {{"1", {1, 22}}, {"2", {"string_view"sv}}}}};

            for (const auto& ps : parameters)
            {
                const auto id{std::get<std::string_view>(ps)};
                const auto match{std::find_if(
                    handlers.cbegin(),
                    handlers.cend(),
                    [&id](const auto& handler) {
                        return id == std::get<std::string_view>(handler);
                    })};
                if (match != handlers.cend())
                {
                    try
                    {
                        std::get<handler> (*match)(std::get<params>(ps));
                    }
                    catch (const std::exception& e)
                    {
                        std::cout << e.what() << '\n';
                    }
                }
            }
        }
    }
}
