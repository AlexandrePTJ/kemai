#include "applicationContext.h"

namespace kemai
{
    ApplicationContext::ApplicationContext(QObject *parent):
    QObject(parent)
    {
    }

    ApplicationContext::~ApplicationContext() = default;

    void ApplicationContext::initialize()
    {
    }

    void ApplicationContext::login(const QString &host, const QString &token)
    {
    }

} // namespace kemai