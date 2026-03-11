// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#define CATCH_CONFIG_RUNNER
#include <catch2/catch_session.hpp>

#include <QCoreApplication>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    return Catch::Session().run(argc, argv);
}
