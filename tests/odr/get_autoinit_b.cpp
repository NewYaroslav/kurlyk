#include <kurlyk.hpp>
#include <memory>

extern "C" kurlyk::startup::AutoInitializer* get_auto_init_b() {
    return std::addressof(kurlyk::startup::_kurlyk_auto_initializer);
}
