#ifndef BUDOVYSEARCHFORM_H
#define BUDOVYSEARCHFORM_H

#include <QWidget>

#include "ui_budovysearchform.h"

class BudovySearchForm : public QWidget, private Ui::BudovySearchForm
{
  Q_OBJECT
  
public:
  explicit BudovySearchForm( QWidget *parent = 0 );
  ~BudovySearchForm();

  void setZpusobVyuzitiModel( QAbstractItemModel *model );

  QString domovniCislo() { return cisloDomovniLineEdit->text().trimmed(); }
  QString naParcele() { return naParceleLineEdit->text().trimmed(); }
  QString zpusobVyuzitiKod();
  QString lv() { return lvBudovyLineEdit->text().trimmed(); }

protected:

  
private:
    QAbstractItemModel *mZpusobVyuzitiModel;
};

#endif // BUDOVYSEARCHFORM_H
