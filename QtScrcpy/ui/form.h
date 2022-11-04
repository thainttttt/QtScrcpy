#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include <unordered_map>

#include "videoform.h"
#include "../QtScrcpyCore/include/QtScrcpyCore.h"

namespace Ui
{
  class Form;
}

class Form : public QWidget
{
  Q_OBJECT

public:
  explicit Form(int maxRow, QWidget *parent = nullptr);
  ~Form();

  void setRow(int row);
  void addForm(QWidget *w);
  void resetForm();
  void setCurrentForm(QString &serial);
  // true of show mainform, otherwise false
  void updateMainForm(QString& serial);

protected:
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;

private:
  void setMainForm(QWidget *w);
  void unsetMainForm();
  void installShortcut();

public:
  std::unordered_map<std::string, VideoForm*> videoForms;

private:
  // ui
  Ui::Form *ui;
  std::vector<QPoint> recPoints;
  int _maxRow;
  QString currentForm;
  QString mainSerial;
};

#endif // FORM_H
