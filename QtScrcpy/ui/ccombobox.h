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
    CComboBox(QTabWidget* _tabs, bool _all, QWidget* _parent = 0) : QComboBox(_parent)
    {
        tabs=_tabs;
        all = _all;
    }

    ~CComboBox()
    {
    }

    void showPopup()
    {
        if (!all) return;
        clear();
        for (int i=1; i<tabs->count(); i++) {  // start from 1, Ungroup
            addItem(tabs->tabText(i));
        }
        // for (auto portNum : cameras)
        // 	d->CameraPortComboBox->addItem(portNum);

        QComboBox::showPopup();
    }

private:
    QTabWidget* tabs;
    bool all;  // if combobox in all tab, allow setup group

};

#endif // QCCOMBOBOX

