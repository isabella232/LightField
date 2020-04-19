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

    "_", "\\", "|", "~", "<", ">", "=","$", "@", "\"",
    "123", ".", ",", "?", "!", "'","/",":",";", "back\nspace",
    "ABC", ",", "space", "."
};

QStringList all_non_repetable_keys = {"123", "ABC", "#+=", "space"};

int nbkey[KEYS_TYPE];

// In witch row are the key... (there's 4 rows )
const int row_keymap[] = {
    0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,
    3,3,3,3,3
};


Keyboard::Keyboard(QWidget *p) : QWidget(p)
{
    currentKey = 0x0;
    currentindexkeyboard = LOWERCASE;
    uppercase = false;

    initTooltip();

    keys = QVector<QVector< key * > >(KEYS_TYPE);
    nbkey[LOWERCASE] = sizeof(en_lower_keymap)/sizeof(char *);
    initKeys(LOWERCASE,   en_lower_keymap);
    nbkey[NUMBER] = sizeof(en_number_keymap)/sizeof(char *);
    initKeys(NUMBER,      en_number_keymap);
    nbkey[UPPERCASE] = sizeof(en_upper_keymap)/sizeof(char *);
    initKeys(UPPERCASE,   en_upper_keymap);
    nbkey[PUNCTUATION] = sizeof(en_punctuation_keymap)/sizeof(char *);
    initKeys(PUNCTUATION, en_punctuation_keymap);
}

void Keyboard::initKeys( int indexArraykeys, const char *keymap[])
{
    int xCoor = 0;
    int yCoor = 0;

    keys[indexArraykeys] = QVector< key * >(nbkey[indexArraykeys]);

    for(int n=0; n<nbkey[indexArraykeys]; n++)
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
            //Special cases of icons
            if (0 == strcasecmp( keymap[n], "back\nspace")) keys[indexArraykeys][n]->setIconFile(":/images/backspace-white.png");
            if (0 == strcasecmp( keymap[n], "caps"))        keys[indexArraykeys][n]->setIconFile(":/images/shift-white.png");
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
        disconnectKeyRepetition(currentKey);
        this->repaint();
    }
    setKeyPressed( findKey(pos), pos );
}

void Keyboard::mouseReleaseEvent(QMouseEvent *e)
{
   // qDebug() <<  "Mouse release event";
    QPoint pos = e->pos();
    tooltip->hide();

    key *k= findKey( pos );
    if (k != 0x0 )
    {
        disconnectKeyRepetition(k);
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
                if(uppercase == true){
                    uppercase=false;
                    currentindexkeyboard = LOWERCASE;
                }
                repaint();
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

    if(isKeyRepetable(k) && (k->repetionActive == false)){
        QTimer *timer = new QTimer( this );
        QObject::connect( timer, &QTimer::timeout, this, [=]() {  this->checkKeyStillPressed(k); } );
        timer->setInterval(KeyboardRepeatDelayStart); //repetition start delay
        timer->setSingleShot( false );
        timer->setTimerType( Qt::PreciseTimer );
        timer->start( );
        k->repetitionTimer = timer;
        k->repetionActive = true;
    }

}

void Keyboard::checkKeyStillPressed(key *repetitionKey)
{
    if(repetitionKey->pressed && isKeyRepetable(repetitionKey)){
        if ( repetitionKey->text =="back\nspace" )
        {
            emit backspacePressed();
        }else{
        emit keyPressed(repetitionKey->text);
        }
        //switch timer to repetition interval
        repetitionKey->repetitionTimer->setInterval(KeyboardRepeatDelay);
    }

}

bool Keyboard::isKeyRepetable(key *keyCheck){
    if(keyCheck == nullptr) return 0;
    return !(all_non_repetable_keys.contains(keyCheck->text));
}

void Keyboard::disconnectKeyRepetition(key *activeKey){

    if(activeKey != nullptr){
        if(activeKey->repetionActive){
            //qDebug() <<  "Timer disconnection";
            QObject::disconnect( activeKey->repetitionTimer, nullptr, this, nullptr );
            activeKey->repetitionTimer->stop( );
            activeKey->repetitionTimer->deleteLater( );
            activeKey->repetionActive = false;
        }
    }
}

void Keyboard::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    foreach (key *k, keys[currentindexkeyboard])
    {
        k->draw(&painter,style());
    }
}
