#include "fontAwesomeHolder.h"

#include <memory>

using namespace kemai;

fa::QtAwesome* FontAwesomeHolder::get()
{
    static std::unique_ptr<fa::QtAwesome> _instance = std::make_unique<fa::QtAwesome>();
    return _instance.get();
}
