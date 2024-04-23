#include "loginDialog.h"
#include "ui_loginDialog.h"

#include <QMovie>

using namespace kemai;

LoginDialog::LoginDialog(QWidget* parent) : QDialog(parent), m_ui(std::make_unique<Ui::LoginDialog>())
{
    m_ui->setupUi(this);

    auto movie = new QMovie(":/icons/kimai-animated");
    m_ui->lbLogo->setMovie(movie);
    movie->start();
}

LoginDialog::~LoginDialog() = default;
