#ifndef __INPUTDIALOG_H__
#define __INPUTDIALOG_H__

#include <iostream>
#include <cstdbool>
#include "keyboard.h"
#include "window.h"
#include "utils.h"

class InputDialog: public QDialog
{
    Q_OBJECT

private:
        Keyboard*    _keyboard          { new Keyboard(this) };
        QLabel*      _message           { new QLabel };
        QLineEdit*   _input             { new QLineEdit };
        QPushButton* _okButton          { new QPushButton("Ok") };
        QPushButton* _cancelButton      { new QPushButton("Cancel")};
        QWidget*     _widget            { new QWidget };
public:
        InputDialog() { }
        InputDialog(QString text) {
            auto origFont    = font( );
            auto fontAwesome = ModifyFont( origFont, "Font Awesome 5 Brands", LargeFontSize );

            Window* win = App::mainWindow();
            QRect r = win->geometry();

            move(r.x()+100, r.y()+100);
            resize(824, 400);

            _message ->setText(text);
            _keyboard->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
            _message->setFont(fontAwesome);
            _okButton->setFont(fontAwesome);
            _cancelButton->setFont(fontAwesome);
            _input->setFont(fontAwesome);

            _okButton->setMinimumSize(QSize(245, 70));
            _cancelButton->setMinimumSize(QSize(245, 70));
            setContentsMargins( { } );
            setMinimumSize(QSize(580, 355));

            setLayout(
                WrapWidgetsInVBox(
                    _message,
                    _input,
                    _keyboard,
                    WrapWidgetsInHBox(_okButton, _cancelButton)
                )
            );

            setModal(true);

            // Window backgorund
            // TODO: make it transparent
            _widget->setStyleSheet("background-color: rgba(255, 255, 255, 10%);");
            _widget->setFixedSize(QSize(1024,600));
            _widget->setWindowOpacity(0.5);
            _widget->show();

            QWidget::connect(_keyboard, &Keyboard::keyPressed, this, &InputDialog::keyPressed);
            QWidget::connect(_keyboard, &Keyboard::backspacePressed, this, &InputDialog::backspacePressed);
            QWidget::connect(_okButton, &QPushButton::clicked, this, &InputDialog::oklCLicked_clicked);
            QWidget::connect(_cancelButton, &QPushButton::clicked, this, &InputDialog::cancelCLicked_clicked);
        }

        ~InputDialog() {
            delete _keyboard;
            delete _message;
            delete _input;
            delete _okButton;
            delete _cancelButton;
            delete _widget;
        }

        QString getValue() {
            return _input->text();
        }

    public slots:
        void keyPressed( QString t) {
            int cursorPos = _input->cursorPosition();
           _input->setText(_input->text().insert(cursorPos, t));
           _input->setCursorPosition(cursorPos+1);
        }

        void backspacePressed(  ) {
            int cursorPos = _input->cursorPosition();
            QString text = _input->text();
            if(cursorPos == 0)
                return;
            _input->setText(text.remove(cursorPos-1, 1));
            _input->setCursorPosition(cursorPos-1);
        }

        void cancelCLicked_clicked(bool) {
            _widget->hide();
            this->setResult(QDialog::Rejected);
            this->reject();
            this->close();
        }

        void oklCLicked_clicked(bool) {
            _widget->hide();
            this->setResult(QDialog::Accepted);
            this->accept();
            this->close();
        }



    signals:
        void okclicked();
};

#endif // __INPUTDIALOG_H__
