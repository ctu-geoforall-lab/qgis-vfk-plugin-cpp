#ifndef PARCELYSEARCHFORM_H
#define PARCELYSEARCHFORM_H

#include "ui_parcelysearchform.h"

class ParcelySearchForm : public QWidget, private Ui::ParcelySearchForm
{
  Q_OBJECT

public:
  explicit ParcelySearchForm(QWidget *parent = 0);

  void postInit();

  void setDruhPozemkuModel( QAbstractItemModel *model );
  void setDruhPozemkuStavebniModel( QAbstractItemModel *model );
  void setDruhPozemkuPozemkovaModel( QAbstractItemModel *model );

  QString parcelniCislo() { return parCisloLineEdit->text().trimmed(); }
  int typParcely();
  QString druhPozemkuKod();
  QString lv() { return lvParcelyLineEdit->text().trimmed(); }


signals:
  void searchEnabled( bool enable );

private slots:
  void parcelySearchEnabled();
  void setDruhModel();

private:
  QAbstractItemModel *defaultModel;
  QAbstractItemModel *stavebniModel;
  QAbstractItemModel *pozemkovaModel;
};

#endif // PARCELYSEARCHFORM_H
