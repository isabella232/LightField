#ifndef __PRINTPROFILE_H__
#define __PRINTPROFILE_H__

class PrintProfile: public QObject {

    Q_OBJECT;

public:

    PrintProfile( QObject* parent = nullptr ): QObject( parent ) {
        /*empty*/
    }

    virtual ~PrintProfile( ) override {
        /*empty*/
    }

    QString const& profileName( ) const {
        return _name;
    }

    void setProfileName( QString const& newName ) {
        emit profileNameChanged( newName );
        _name = newName;
    }

protected:

private:

    QString _name;

signals:
    ;

    void profileNameChanged( QString const& newName );

public slots:
    ;

protected slots:
    ;

private slots:
    ;

};

#endif // !__PRINTPROFILE_H__
