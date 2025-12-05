#include "BoardEditor.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>

#include <fstream>

bool BoardEditor::editBoard(QWidget *parent,
                            int snakes[][2], int &numSnakes, int maxSnakes,
                            int ladders[][2], int &numLadders, int maxLadders,
                            QTextEdit *logText)
{
    QDialog dlg(parent);
    dlg.setWindowTitle("Custom Board Editor");

    QVBoxLayout *layout = new QVBoxLayout(&dlg);

    QLabel *label = new QLabel(
        "Enter snakes and ladders (one per line):\n"
        "Format: S,head,tail  (head > tail)\n"
        "        L,bottom,top (top > bottom)");
    QTextEdit *editor = new QTextEdit();

    QPushButton *okBtn = new QPushButton("Apply");
    QPushButton *cancelBtn = new QPushButton("Cancel");

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);

    layout->addWidget(label);
    layout->addWidget(editor);
    layout->addLayout(btnLayout);

    QObject::connect(okBtn,    &QPushButton::clicked, &dlg, &QDialog::accept);
    QObject::connect(cancelBtn,&QPushButton::clicked, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted)
        return false;

    QString text = editor->toPlainText();
    QStringList lines = text.split('\n', Qt::SkipEmptyParts);

    numSnakes  = 0;
    numLadders = 0;

    for (const QString &line : lines)
    {
        QStringList parts = line.split(',', Qt::SkipEmptyParts);
        if (parts.size() != 3) continue;

        QString type = parts[0].trimmed().toUpper();
        int a = parts[1].toInt();
        int b = parts[2].toInt();

        // Snake: S, head, tail -> must have head > tail
        if (type == "S" && numSnakes < maxSnakes)
        {
            if (a > b)
            {
                snakes[numSnakes][0] = a;
                snakes[numSnakes][1] = b;
                numSnakes++;
            }
            else
            {
                if (logText)
                    logText->append(
                        QString("Ignored invalid snake: S,%1,%2 (head must be > tail).")
                            .arg(a).arg(b));
            }
        }
        // Ladder: L, bottom, top -> must have top > bottom
        else if (type == "L" && numLadders < maxLadders)
        {
            if (b > a)
            {
                ladders[numLadders][0] = a;
                ladders[numLadders][1] = b;
                numLadders++;
            }
            else
            {
                if (logText)
                    logText->append(
                        QString("Ignored invalid ladder: L,%1,%2 (top must be > bottom).")
                            .arg(a).arg(b));
            }
        }
    }

    return true;
}

void BoardEditor::saveBoardToCsv(const char *filename,
                                 const int snakes[][2], int numSnakes,
                                 const int ladders[][2], int numLadders)
{
    std::ofstream file(filename);
    if (!file.is_open())
        return;

    for (int i = 0; i < numSnakes; ++i)
        file << "S," << snakes[i][0] << "," << snakes[i][1] << "\n";
    for (int i = 0; i < numLadders; ++i)
        file << "L," << ladders[i][0] << "," << ladders[i][1] << "\n";

    file.close();
}
