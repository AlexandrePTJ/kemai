/*
 * Copyright Kemai authors
 * Distributed under the MIT License.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

// Headers
#include <memory>

#include <QDialog>

// Fwd Decl
namespace Ui {
class LoginDialog;
}

// Class Decl
namespace kemai {
class LoginDialog : public QDialog
{
public:
    LoginDialog(QWidget* parent = nullptr);
    ~LoginDialog() override;
    
private:
    std::unique_ptr<Ui::LoginDialog> m_ui;
};
} // namespace kemai
