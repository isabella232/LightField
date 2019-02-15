#ifndef __BEDHEIGHTADJUSTMENT_H__
#define __BEDHEIGHTADJUSTMENT_H__

class BedHeightAdjustmentDialog: public QDialog {

    Q_OBJECT

public:

    BedHeightAdjustmentDialog( QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags( ) );
    virtual ~BedHeightAdjustmentDialog( ) override;

    double newBedHeight( ) const { return _newBedHeight; }

protected:

private:

    QLabel*      _bedHeightLabel    { new QLabel      };
    QLineEdit*   _bedHeightLineEdit { new QLineEdit   };
    QHBoxLayout* _bedHeightHBox     { new QHBoxLayout };
    QPushButton* _okButton          { new QPushButton };
    QPushButton* _cancelButton      { new QPushButton };
    QHBoxLayout* _buttonsHBox       { new QHBoxLayout };
    QVBoxLayout* _layout            { new QVBoxLayout };

    double       _newBedHeight      {                 };

signals:

public slots:

protected slots:

private slots:

    void _bedHeightLineEdit_editingFinished( );
    void _okButton_clicked( bool checked );
    void _cancelButton_clicked( bool checked );

};

#endif // __BEDHEIGHTADJUSTMENT_H__
