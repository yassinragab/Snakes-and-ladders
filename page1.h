#ifndef PAGE1_H
#define PAGE1_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>

class WelcomePage : public QWidget {
    Q_OBJECT
public:
    explicit WelcomePage(QWidget *parent = nullptr);

signals:
    void startGame();  // Emitted when user clicks "Start Game"

private slots:
    void onPlayButtonClicked();

private:
    QPushButton *playButton;
    QLabel *titleLabel;
};

#endif // PAGE1_H
