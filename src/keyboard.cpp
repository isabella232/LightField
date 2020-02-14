#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include "key.h"
#include "keyboard.h"


// Declaration off the differente keys...
const char *en_lower_keymap[] = {
    "q", "w", "e", "r", "t", "y", "u", "i", "o", "p",
    "a", "s", "d", "f", "g", "h", "j", "k", "l",";",
    "caps", "z", "x", "c", "v", "b", "n", "m",",", "back\nspace",
    "123", ".", "space", "@"
};

const char *en_upper_keymap[] = {
    "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",
    "A", "S", "D", "F", "G", "H", "J", "K", "L",";",
    "caps", "Z", "X", "C", "V", "B", "N", "M",",", "back\nspace",
    "123", ".", "space", "@"
};

const char *en_number_keymap[] = {
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
    "-", "/", ":", ";", "(", ")", "€", "&", "@", "\"",
    "#+=", ".", ",", "?", "!", "'", "+","\\","%","back\nspace",
    "ABC", ",", "space", "."
};

const char *en_punctuation_keymap[] = {
    "[", "]", "{", "}", "#", "%", "^", "*", "+", "=",
    "_", "\\", "|", "~", "<", ">", "=","$", "@", "\"",
    "123", ".", ",", "?", "!", "'","/",":",";", "back\nspace",
    "ABC", ",", "space", "."
};

// In witch row are the key... (there's 4 rows )
const int row_keymap[] = {
    0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,
    3,3,3,3,3
};


const int nbkey = sizeof(en_lower_keymap)/sizeof(char *);

Keyboard::Keyboard(QWidget *p) : QWidget(p)
{
    currentKey = 0x0;
    currentindexkeyboard = LOWERCASE;
    uppercase = false;

    initTooltip();

    keys = QVector<QVector< key * > >(KEYS_TYPE);
    for (int n=0;n < KEYS_TYPE ; n++)
    {
      keys[n] = QVector< key * >(nbkey);
    }

    initKeys(LOWERCASE,   en_lower_keymap);
    initKeys(NUMBER,      en_number_keymap);
    initKeys(UPPERCASE,   en_upper_keymap);
    initKeys(PUNCTUATION, en_punctuation_keymap);
}

void Keyboard::initKeys( int indexArraykeys, const char *keymap[])
{
    int xCoor = 0;
    int yCoor = 0;

    for(int n=0; n<nbkey; n++)
    {
        keys[indexArraykeys][n] = new key(keymap[n]);
       // qDebug() <<  "n="<< n;

        if ( n > 0 )
        {
            // Special length button
            if (0 == strcasecmp( keymap[n], "space"    )) keys[indexArraykeys][n]->W=574;

            // Calculat e new ccooridinate of button
            if (row_keymap[n-1]!=row_keymap[n])
            {
                xCoor = 2;
            }
            else
            {
                xCoor = keys[indexArraykeys][n-1]->X + keys[indexArraykeys][n-1]->W;
            }
            yCoor = row_keymap[n] * DEFAULT_YSIZE_BUTTON;
        }
        else
        {
            xCoor = 2;
            yCoor = 0;
        }

        keys[indexArraykeys][n]->setX(xCoor);
        keys[indexArraykeys][n]->setY(yCoor);

        //qDebug() <<"N: " << n << "\t\t" <<keys[indexArraykeys][n]->text << "\t\t W= " << keys[indexArraykeys][n]->W;
    }
}

void Keyboard::initTooltip()
{
    tooltip = new QLabel("");
    tooltip->setWindowFlags( Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint );

    QFont serifFont("Times", 18, QFont::Bold);
    tooltip->setFont(serifFont);
    tooltip->setAlignment(Qt::AlignCenter);

    // Bug fix of first pres button
    tooltip->show();
    tooltip->hide();
}

void Keyboard::mousePressEvent(QMouseEvent * e)
{
    //qDebug() <<  "Mouse press event";
    QWidget::mousePressEvent(e);

    QPoint pos = e->pos();
    setKeyPressed( findKey(pos), pos );
}

void Keyboard::mouseMoveEvent(QMouseEvent * e)
{
    //qDebug() <<  "Mouse move event";
    QPoint pos = e->pos();

    if (currentKey != 0x0 && !currentKey->getRect().contains(pos))
    {
        tooltip->hide();
        currentKey->setPressed(false);
        this->repaint();
    }
    setKeyPressed( findKey(pos), pos );
}

void Keyboard::mouseReleaseEvent(QMouseEvent *e)
{
    //qDebug() <<  "Mouse release event";
    QPoint pos = e->pos();
    tooltip->hide();

    key *k= findKey( pos );
    if (k != 0x0 )
    {
        if ( k->text == "ABC")
        {
            currentindexkeyboard = LOWERCASE;
            repaint();
            return;
        }
        if (k->text== "#+=" )
        {
            currentindexkeyboard = PUNCTUATION;
            repaint();
            return;
        }

        if (k->text=="caps")
        {
            if ( uppercase ==false)
            {
               currentindexkeyboard = UPPERCASE;
               uppercase = true;
            }
            else
            {
                currentindexkeyboard = LOWERCASE;
                uppercase = false;
            }
            repaint();
            return;
        }

        if (k->text=="123")
        {
            currentindexkeyboard = NUMBER;
            repaint();
        }
        else
        {
            if ( k->text =="back\nspace" )
            {
                emit backspacePressed();
                return;
            }
            else if (k->text=="enter")
            {
                emit returnPressed();
                return;
            }
            else if ( k->text == "space")
            {
                emit keyPressed(" ");
            }
            else
            {
                emit keyPressed(k->text);
            }
        }
    }
}

key *Keyboard::findKey(QPoint p)
{
    foreach (key *k, keys[currentindexkeyboard])
    {
      if (k->getRect().contains(p))
      {
          return k;
      }
    }
    return 0x0;
}

void Keyboard::setKeyPressed( key *k, QPoint /*pos*/)
{
    currentKey = k;
    if (k == 0x0) return;

    k->setPressed(true);
    this->repaint();

    QPoint p = QWidget::mapToGlobal(this->pos() +QPoint( k->X, k->Y));
    tooltip->setGeometry(p.x(),p.y()-50,50, 50);
    tooltip->setText(k->text);
    tooltip->show(); // this line makes bug with first relase event
}

void Keyboard::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    foreach (key *k, keys[currentindexkeyboard])
    {
        k->draw(&painter,style());
    }
}
