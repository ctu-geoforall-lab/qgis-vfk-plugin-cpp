#ifndef VLASTNICISEARCHFORM_H
#define VLASTNICISEARCHFORM_H

#include "ui_vlastnicisearchform.h"

class VlastniciSearchForm : public QWidget, private Ui::VlastniciSearchForm
{
  Q_OBJECT

public:
  explicit VlastniciSearchForm(QWidget *parent = 0);

  QString jmeno() { return jmenoLineEdit->text().trimmed(); }
  QString rcIco() { return rcIcoLineEdit->text().trimmed(); }
  bool isSjm() { return sjmCheckBox->isChecked(); }
  bool isOpo() { return opoCheckBox->isChecked(); }
  bool isOfo() { return ofoCheckBox->isChecked(); }
  QString lv() { return lvVlastniciLineEdit->text().trimmed(); }

signals:
  void searchEnabled( bool enable );

private slots:
  void vlastniciSearchEnabled();
  void vlastniciSetRcIcoEnabled();


};

#endif // VLASTNICISEARCHFORM_H
