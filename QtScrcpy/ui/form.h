#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include <unordered_map>

#include "videoform.h"

namespace Ui
{
  class Form;
}

class Form : public QWidget
{
  Q_OBJECT

public:
  explicit Form(QWidget *parent = nullptr);
  ~Form();

  void setRow(int row);
  void setMainForm(QWidget *w);
  void unsetMainForm();
  void addForm(QWidget *w);
  void resetForm();

  void setCurrentForm(QString &serial);

protected:
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;

public:
  std::unordered_map<std::string, VideoForm*> videoForms;

private:
  // ui
  Ui::Form *ui;
  std::vector<QPoint> recPoints;
  int _maxRow=2;
  QString currentForm;
};

#endif // FORM_H
