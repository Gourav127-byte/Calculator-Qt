// main.cpp
#include <QApplication>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QChar>
#include <QDebug>

class Calculator : public QWidget {
    Q_OBJECT
public:
    Calculator(QWidget *parent = nullptr) : QWidget(parent),
        display(new QLineEdit("0")),
        waitingForOperand(true),
        storedOperand(0.0),
        pendingOperator('\0')
    {
        display->setReadOnly(true);
        display->setAlignment(Qt::AlignRight);
        display->setMaxLength(24);
        display->setFixedHeight(40);
        QFont f = display->font();
        f.setPointSize(14);
        display->setFont(f);

        // Buttons text in grid
        QString buttons[4][4] = {
            {"7","8","9","/"},
            {"4","5","6","*"},
            {"1","2","3","-"},
            {"0",".","=","+"}
        };

        auto *grid = new QGridLayout;
        for (int r=0; r<4; ++r) {
            for (int c=0; c<4; ++c) {
                QPushButton *btn = new QPushButton(buttons[r][c]);
                btn->setFixedSize(60,48);
                grid->addWidget(btn, r, c);
                connect(btn, &QPushButton::clicked, this, [this, btn](){ onButtonClicked(btn->text()); });
            }
        }

        // Clear button
        QPushButton *clearBtn = new QPushButton("C");
        clearBtn->setFixedHeight(40);
        connect(clearBtn, &QPushButton::clicked, this, &Calculator::onClear);

        auto *vbox = new QVBoxLayout;
        vbox->addWidget(display);
        vbox->addLayout(grid);
        vbox->addWidget(clearBtn);
        setLayout(vbox);
        setWindowTitle("Qt Calculator");
        setFixedSize(sizeHint());
    }

private slots:
    void onButtonClicked(const QString &text) {
        if (text >= "0" && text <= "9") {
            digitClicked(text);
            return;
        }
        if (text == ".") {
            pointClicked();
            return;
        }
        if (text == "=") {
            equalClicked();
            return;
        }
        // operators: + - * /
        if (text == "+" || text == "-" || text == "*" || text == "/") {
            operatorClicked(text.at(0));
            return;
        }
    }

    void onClear() {
        display->setText("0");
        waitingForOperand = true;
        storedOperand = 0.0;
        pendingOperator = '\0';
    }

private:
    QLineEdit *display;
    bool waitingForOperand;
    double storedOperand;
    QChar pendingOperator;

    void digitClicked(const QString &digit) {
        if (display->text() == "0" && digit == "0")
            return;
        if (waitingForOperand) {
            display->setText(digit);
            waitingForOperand = false;
        } else {
            display->setText(display->text() + digit);
        }
    }

    void pointClicked() {
        if (waitingForOperand) {
            display->setText("0.");
            waitingForOperand = false;
        } else if (!display->text().contains('.')) {
            display->setText(display->text() + ".");
        }
    }

    void operatorClicked(QChar op) {
        double operand = display->text().toDouble();
        if (!pendingOperator.isNull()) {
            // perform previous pending operation first
            if (!performOperation(operand, pendingOperator)) {
                display->setText("Error");
                pendingOperator = '\0';
                waitingForOperand = true;
                return;
            }
            display->setText(QString::number(storedOperand));
        } else {
            storedOperand = operand;
        }
        pendingOperator = op;
        waitingForOperand = true;
    }

    void equalClicked() {
        double operand = display->text().toDouble();
        if (!pendingOperator.isNull()) {
            if (!performOperation(operand, pendingOperator)) {
                display->setText("Error");
                pendingOperator = '\0';
                waitingForOperand = true;
                return;
            }
            display->setText(QString::number(storedOperand));
            pendingOperator = '\0';
        } else {
            // no pending operator — keep the displayed value
        }
        waitingForOperand = true;
    }

    bool performOperation(double rightOperand, QChar op) {
        if (op == '+') storedOperand = storedOperand + rightOperand;
        else if (op == '-') storedOperand = storedOperand - rightOperand;
        else if (op == '*') storedOperand = storedOperand * rightOperand;
        else if (op == '/') {
            if (qFuzzyCompare(rightOperand + 1.0, 1.0)) { // rightOperand == 0?
                return false; // divide by zero
            }
            storedOperand = storedOperand / rightOperand;
        } else {
            return false;
        }
        return true;
    }
};

#include "main.moc"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    Calculator calc;
    calc.show();
    return app.exec();
}

