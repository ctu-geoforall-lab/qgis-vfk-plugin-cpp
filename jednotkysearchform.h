#ifndef JEDNOTKYSEARCHFORM_H
#define JEDNOTKYSEARCHFORM_H

#include "ui_jednotkysearchform.h"

class JednotkySearchForm : public QWidget, private Ui::JednotkySearchForm
{
  Q_OBJECT
  
public:
  explicit JednotkySearchForm(QWidget *parent = 0);

  void setZpusobVyuzitiModel( QAbstractItemModel *model );

  QString cisloJednotky() { return mCisloJednotkyLineEdit->text().trimmed(); }
  QString domovniCislo() { return mCisloDomovniLineEdit->text().trimmed(); }
  QString naParcele() { return mNaParceleLineEdit->text().trimmed(); }
  QString zpusobVyuzitiKod();
  QString lv() { return mLvJednotkyLineEdit->text().trimmed(); }

private:
  QAbstractItemModel *mZpusobVyuzitiModel;
};

#endif // JEDNOTKYSEARCHFORM_H
