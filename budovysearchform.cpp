#include "budovysearchform.h"

#include <QDebug>

BudovySearchForm::BudovySearchForm(QWidget *parent) :
  QWidget(parent)
{
  setupUi(this);
}

BudovySearchForm::~BudovySearchForm()
{
}

void BudovySearchForm::setZpusobVyuzitiModel( QAbstractItemModel *model )
{
  mZpusobVyuzitiModel = model;
  mZpVyuzitiCombo->setModel( model );
  mZpVyuzitiCombo->setModelColumn( 1 ); // FIXME: magic number
}

QString BudovySearchForm::zpusobVyuzitiKod()
{
  int row = mZpVyuzitiCombo->currentIndex();
  QModelIndex index = mZpVyuzitiCombo->model()->index( row, 0 ); // FIXME: magic number
  return mZpVyuzitiCombo->model()->data( index ).toString();
}

