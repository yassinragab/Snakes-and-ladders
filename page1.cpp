#include "Page1.h"
#include <QPixmap>
#include <QPalette>
#include <QLinearGradient>
#include <qboxlayout.h>

WelcomePage::WelcomePage(QWidget *parent) : QWidget(parent), playButton(nullptr), titleLabel(nullptr) {
    setWindowTitle("Snakes & Ladders");
    setFixedSize(1200, 700);

    // FIXED: Add background gradient
    QPalette palette;
    QLinearGradient gradient(0, 0, 0, 700);
    gradient.setColorAt(0, QColor(34, 139, 34));    // Forest green
    gradient.setColorAt(0.3, QColor(50, 205, 50));  // Lime green
    gradient.setColorAt(0.7, QColor(0, 100, 0));    // Dark green
    gradient.setColorAt(1, QColor(20, 60, 20));     // Deep green
    palette.setBrush(QPalette::Window, QBrush(gradient));
    setPalette(palette);
    setAutoFillBackground(true);

    // Title
    titleLabel = new QLabel("🎲 SNAKES & LADDERS 🎲", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "font-size: 48px; font-weight: bold; color: #FFD700; padding: 20px; "
        "background: rgba(0,0,0,0.3); border-radius: 20px;"
        );

    // BIG Description Box
    QLabel *description = new QLabel(this);
    description->setFixedSize(650, 140);
    description->setWordWrap(true);
    description->setAlignment(Qt::AlignCenter);
    description->setText(
        "🎯 Navigate from square 1 to 100!\n"
        "🐍 Slide down snakes • ⬆️ Climb ladders\n"
        "✨ Multiplayer, Shortest Path, Board Editor"
        );
    description->setStyleSheet(
        "font-size: 18px; color: #F0F8FF; padding: 30px; "
        "background: rgba(0,0,0,0.3); border-radius: 25px; "
        "border: 2px solid rgba(255,215,0,0.4);"
        );

    // Play Button (FIXED styling)
    playButton = new QPushButton("▶ START GAME", this);
    playButton->setFixedSize(280, 90);
    playButton->setStyleSheet(
        "QPushButton { "
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #32CD32, stop:1 #006400); "
        "border: 3px solid #FFD700; color: white; font-size: 28px; font-weight: bold; "
        "border-radius: 50px;"
        "} "
        "QPushButton:hover { "
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3CB371, stop:1 #228B22); "
        "border: 3px solid #FFA500;"
        "} "
        "QPushButton:pressed { "
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #228B22, stop:1 #006400);"
        "}"
        );

    // Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(60, 40, 60, 40);
    layout->setSpacing(30);
    layout->addStretch(1);
    layout->addWidget(titleLabel, 0, Qt::AlignCenter);
    layout->addWidget(description, 0, Qt::AlignCenter);
    layout->addStretch(1);
    layout->addWidget(playButton, 0, Qt::AlignCenter);
    layout->addStretch(2);

    // CRITICAL: Connect button
    connect(playButton, &QPushButton::clicked, this, &WelcomePage::onPlayButtonClicked);
}

void WelcomePage::onPlayButtonClicked() {
    emit startGame();
    hide();
}
