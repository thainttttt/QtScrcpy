#ifndef QCCOMBOBOX
#define QCCOMBOBOX

#include <QWidget>
#include <QComboBox>

/**
 * @brief QComboBox with support of checkboxes
 * http://stackoverflow.com/questions/8422760/combobox-of-checkboxes
 */
class CComboBox : public QComboBox
{
    Q_OBJECT

public:
    CComboBox(QTabWidget* _tabs, QWidget* _parent = 0) : QComboBox(_parent)
    {
        tabs=_tabs;
    }

    ~CComboBox()
    {
    }

    void showPopup()
    {
        clear();
        for (int i=0; i<tabs->count(); i++) {
            addItem(tabs->tabText(i));
        }
        // for (auto portNum : cameras)
        // 	d->CameraPortComboBox->addItem(portNum);

        QComboBox::showPopup();
    }

private:
    QTabWidget* tabs;

};

#endif // QCCOMBOBOX

